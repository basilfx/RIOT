/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net_gnrc_knx_l7
 * @file
 * @brief       Implementation of the KNX Layer 7 stack
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include "net/gnrc/knx_l7.h"
#include "net/knx.h"

#include "knx_device.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Allocate memory for stack
 */
static char _stack[GNRC_KNX_L7_STACK_SIZE + DEBUG_EXTRA_STACKSIZE];

/**
 * @brief   PID of the thread
 */
static kernel_pid_t gnrc_knx_l7_pid = KERNEL_PID_UNDEF;

typedef void (next_cb_t)(knx_device_t *device, gnrc_pktsnip_t *pkt, knx_com_object_t *com_object);

static knx_device_t *device;

static gnrc_pktsnip_t *_send_response_extended(gnrc_pktsnip_t *pkt, knx_apci_extended_t apci, uint8_t *data, size_t len)
{
    /* allocate output telegram */
    gnrc_pktsnip_t *out = gnrc_pktbuf_add(NULL, NULL, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN + len, GNRC_NETTYPE_KNX_L7);

    if (out == NULL) {
        DEBUG("[gnrc_knx_l7] _send_response_extended: unable to allocate telegram\n");
        return NULL;
    }

    /* prepare data */
    knx_addr_t src, dst;

    knx_telegram_get_src_addr(pkt->data, &src);
    knx_telegram_get_dst_addr(pkt->data, &dst);

    knx_telegram_set_type(out->data, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_group_addressed(out->data, false);
    knx_telegram_set_src_addr(out->data, &dst);
    knx_telegram_set_dst_addr(out->data, &src);
    knx_telegram_set_payload_length(out->data, len);
    knx_telegram_set_routing_count(out->data, 6);
    knx_telegram_set_repeated(out->data, false);
    knx_telegram_set_priority(out->data, knx_telegram_get_priority(pkt->data));

    knx_tpci_set(out->data, knx_tpci_get(pkt->data));

#if ENABLE_DEBUG
    char src_str[KNX_ADDR_MAX_STR_LEN], dst_str[KNX_ADDR_MAX_STR_LEN];

    knx_addr_physical_to_str(src_str, &src, sizeof(src_str));
    knx_addr_physical_to_str(dst_str, &dst, sizeof(dst_str));

    DEBUG("[gnrc_knx_l7] _send_response_extended: src=%s dst=%s\n", src_str, dst_str);
#endif

    memcpy(&((uint8_t *)out->data)[7], data, len);

    knx_apci_extended_set(out->data, apci);

    /* send telegram */
    return out;
}

static gnrc_pktsnip_t *_authorize_request(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    (void)device;

    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 6) {
        DEBUG("[gnrc_knx_l7_apci] _authorize_request: incorrect payload length\n");
        return NULL;
    }

    DEBUG("[gnrc_knx_l7] _authorize_request: level=unused\n");

    uint8_t buf[2] = { 0x00, 0x00 };

    return _send_response_extended(pkt, KNX_APCI_EXTENDED_AUTHORIZE_RESPONSE, buf, 2);
}

static gnrc_pktsnip_t *_property_value_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 5) {
        DEBUG("[gnrc_knx_l7_apci] _property_value_read: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, false);

    uint8_t object = data[0];
    uint8_t id = data[1];
    uint8_t count = data[2] >> 4;
    uint16_t start = ((data[2] & 0x0f) << 4) | data[3]; // TODO

    DEBUG("[gnrc_knx_l7] _property_value_read: object=%d id=%d count=%d start=%d\n", object, id, count, start);

    /* prepare response */
    uint8_t buf[15];

    buf[1] = data[0];
    buf[2] = data[1];
    buf[3] = data[2];
    buf[4] = data[3];

    /* find property */
    knx_property_t *prop = knx_property_find_by_id(device->objects, object, id);

    if (prop == NULL) {
        DEBUG("[gnrc_knx_l7] _property_value_read: not found\n");
        buf[3] = 0;

        return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE, buf, 5);
    }

    /* return number of elements if start is zero, return the property value
       otherwise */
    if (start == 0) {
        buf[5] = 0;
        buf[6] = knx_property_elements(prop);

        return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE, buf, 5 + 2);
    }
    else {
        /* raise property read event */
        knx_event_prop_read_t event = {
            .property = prop,
            .object = object,
            .count = count,
            .start = start
        };

        if (device->event_callback != NULL) {
            device->event_callback(device, KNX_DEVICE_EVENT_PROP_READ, &event);
        }

        /* read property into buffer */
        ssize_t result = knx_property_read(prop, count, start, &buf[5], 10);

        if (result < 0) {
            DEBUG("[gnrc_knx_l7] _property_value_read: error while reading property\n");
            return NULL;
        }

        return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE, buf, 5 + result);
    }
}

static gnrc_pktsnip_t *_property_value_write(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) < 5) {
        DEBUG("[gnrc_knx_l7_apci] _property_value_write: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, false);

    uint8_t object = data[0];
    uint8_t id = data[1];
    uint8_t count = data[2] >> 4;
    uint16_t start = ((data[2] & 0x0f) << 4) | data[3]; // TODO
    uint8_t *payload = &(data[4]);

    DEBUG("[gnrc_knx_l7] _property_value_write: object=%d id=%d count=%d start=%d\n", object, id, count, start);

    /* prepare response */
    uint8_t buf[15];

    buf[1] = data[0];
    buf[2] = data[1];
    buf[3] = data[2];
    buf[4] = data[3];

    /* find property */
    knx_property_t *prop = knx_property_find_by_id(device->objects, object, id);

    if (prop == NULL) {
        DEBUG("[gnrc_knx_l7] _property_value_write: not found\n");
        buf[3] = 0;

        return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE, buf, 5);
    }

    /* read property into buffer */
    ssize_t result = knx_property_write(prop, count, start, payload, knx_telegram_get_payload_length(telegram) - 4);

    if (result < 0) {
        DEBUG("[gnrc_knx_l7] _property_value_write: unable to write property (%d)\n", (int)result);
        return NULL;
    }

    /* raise property write event */
    knx_event_prop_write_t event = {
        .property = prop,
        .object = object,
        .count = count,
        .start = start,
        .data = payload
    };

    if (device->event_callback != NULL) {
        device->event_callback(device, KNX_DEVICE_EVENT_PROP_WRITE, &event);
    }

    /* for response, read property into buffer */
    result = knx_property_read(prop, count, start, &buf[5], 10);

    if (result < 0) {
        DEBUG("[gnrc_knx_l7] _property_write: error while reading property\n");
        return NULL;
    }

    return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE, buf, 5 + result);
}

static gnrc_pktsnip_t *_property_description_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 4) {
        DEBUG("[gnrc_knx_l7_apci] _property_description_read: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, false);

    uint8_t object = data[0];
    uint8_t id = data[1];
    uint16_t index = data[2];

    DEBUG("[gnrc_knx_l7] _property_description_read: object=%d id=%d index=%d\n", object, id, index);

    knx_property_t *prop;

    if (id == 0) {
        prop = knx_property_find_by_index(device->objects, object, index);
    }
    else {
        prop = knx_property_find_by_id(device->objects, object, id);
    }

    /* prepare respone */
    uint8_t buf[15];

    buf[1] = data[0];
    buf[2] = data[1];
    buf[3] = data[2];

    if (prop == NULL) {
        DEBUG("[gnrc_knx_l7] _property_description_read: not found\n");
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0xff;

        return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_DESCRIPTION_RESPONSE, buf, 8);
    }

    buf[2] = prop->id;

    buf[4] = prop->type | ((prop->flags & KNX_PROPERTY_FLAG_WRITABLE) ? 0x80 : 0x00);
    buf[5] = 0;
    buf[6] = knx_property_elements(prop);
    buf[7] = 0xff; /* read and write access level */

    return _send_response_extended(pkt, KNX_APCI_EXTENDED_PROPERTY_DESCRIPTION_RESPONSE, buf, 8);
}

gnrc_pktsnip_t *_individual_addr_serial_write(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 8) {
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, false);

    uint8_t *serial = &(data[0]);
    knx_addr_t *addr = (knx_addr_t *)&(data[6]);

#if ENABLE_DEBUG
    char addr_str[KNX_ADDR_MAX_STR_LEN];

    DEBUG("[gnrc_knx_l7] _individual_addr_serial_write: serial=%02x%02x%02x%02x%02x%02x addr=%s\n", serial[0], serial[1], serial[2], serial[3], serial[4], serial[5], knx_addr_physical_to_str(
              addr_str, addr,
              KNX_ADDR_MAX_STR_LEN));
#endif

    if (memcmp(serial, device->info->serial, 6) != 0) {
        DEBUG("[gnrc_knx_l7] _individual_addr_serial_read: not for us\n");
        return NULL;
    }

    /* delegate to implementation */
    if (device->event_callback != NULL) {
        knx_event_set_address_t event = {
            .address = addr
        };

        device->event_callback(device, KNX_DEVICE_EVENT_SET_ADDRESS, &event);
    }

    return NULL;
}

gnrc_pktsnip_t *_individual_addr_serial_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 7) {
        DEBUG("[gnrc_knx_l7] _individual_addr_serial_read: payload size incorrect\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, false);

    uint8_t *serial = &(data[0]);

    DEBUG("[gnrc_knx_l7] _individual_addr_serial_read: serial=%02x%02x%02x%02x%02x%02x\n", serial[0], serial[1], serial[2], serial[3], serial[4], serial[5]);

    if (memcmp(serial, device->info->serial, 6) != 0) {
        DEBUG("[gnrc_knx_l7] _individual_addr_serial_read: not for us\n");
        return NULL;
    }

    /* prepare response */
    uint8_t buf[15];

    memcpy(&buf[1], serial, 6);
    memset(&buf[7], 0, 4);

    gnrc_pktsnip_t *out = _send_response_extended(pkt, KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_RESPONSE, buf, 11);

    knx_telegram_set_src_addr(out->data, &(device->address));

    return out;
}

gnrc_pktsnip_t *_handle_apci_extended(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_apci_extended_t apci = knx_apci_extended_get(pkt->data);

    DEBUG("[gnrc_knx_l7] _apci_extended: type %04x\n", apci);

    switch (apci) {
        // case KNX_APCI_EXTENDED_DEVICE_DESCRIPTOR_READ:
        //    break;
        // case KNX_APCI_EXTENDED_DEVICE_DESCRIPTOR_RESPONSE:
        //    break;
        // case KNX_APCI_EXTENDED_RESTART:
        //    break;
        case KNX_APCI_EXTENDED_AUTHORIZE_REQUEST:
            return _authorize_request(device, pkt);
        // case KNX_APCI_EXTENDED_AUTHORIZE_RESPONSE:
        //    break;
        case KNX_APCI_EXTENDED_PROPERTY_VALUE_READ:
            return _property_value_read(device, pkt);
        // case KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE:
        //    break;
        case KNX_APCI_EXTENDED_PROPERTY_VALUE_WRITE:
            return _property_value_write(device, pkt);
        case KNX_APCI_EXTENDED_PROPERTY_DESCRIPTION_READ:
            return _property_description_read(device, pkt);
        // case KNX_APCI_EXTENDED_PROPERTY_DESCRIPTION_RESPONSE:
        //    break;
        case KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_READ:
            return _individual_addr_serial_read(device, pkt);
        // case KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_RESPONSE:
        //    break;
        case KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_WRITE:
            return _individual_addr_serial_write(device, pkt);
        default:
            DEBUG("[gnrc_knx_l7] _apci_extended: unhandled\n");
    }

    return NULL;
}

static gnrc_pktsnip_t *_send_response(gnrc_pktsnip_t *pkt, knx_apci_t apci, uint8_t *data, size_t len)
{
    /* allocate output telegram */
    gnrc_pktsnip_t *out = gnrc_pktbuf_add(NULL, NULL, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN + len, GNRC_NETTYPE_KNX_L7);

    if (out == NULL) {
        DEBUG("[gnrc_knx_l7] _send_response: unable to allocate telegram\n");
        return NULL;
    }

    /* prepare data */
    knx_addr_t src, dst;

    knx_telegram_get_src_addr(pkt->data, &dst);
    knx_telegram_get_dst_addr(pkt->data, &src);

    knx_telegram_set_type(out->data, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_group_addressed(out->data, false);
    knx_telegram_set_src_addr(out->data, &src);
    knx_telegram_set_dst_addr(out->data, &dst);
    knx_telegram_set_payload_length(out->data, len);
    knx_telegram_set_routing_count(out->data, 6);
    knx_telegram_set_repeated(out->data, false);
    knx_telegram_set_priority(out->data, knx_telegram_get_priority(pkt->data));

    knx_tpci_set(out->data, knx_tpci_get(pkt->data));

#if ENABLE_DEBUG
    char src_str[KNX_ADDR_MAX_STR_LEN], dst_str[KNX_ADDR_MAX_STR_LEN];

    knx_addr_physical_to_str(src_str, &src, sizeof(src_str));
    knx_addr_physical_to_str(dst_str, &dst, sizeof(dst_str));

    DEBUG("[gnrc_knx_l7] _send_response: src=%s dst=%s\n", src_str, dst_str);
#endif

    memcpy(&((uint8_t *)out->data)[7], data, len);

    knx_apci_set(out->data, apci);

    /* send telegram */
    return out;
}

static gnrc_pktsnip_t *_addr_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    DEBUG("[gnrc_knx_l7] _addr_read: reading address\n");

    if (!device->info->programming_mode) {
        DEBUG("[gnrc_knx_l7] _addr_write: not in programming mode\n");
        return NULL;
    }

    uint8_t buf[] = { 0x00 };

    gnrc_pktsnip_t *out = _send_response(pkt, KNX_APCI_INDIVIDUAL_ADDR_RESPONSE, buf, 1);

    //knx_set_dst_addr(out->data, 0); // Broadcast
    knx_telegram_set_src_addr(out->data, &(device->address));

    return out;
}

static gnrc_pktsnip_t *_addr_write(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 3) {
        DEBUG("[gnrc_knx_l7_apci] _addr_write: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, false);

    knx_addr_t *addr = (knx_addr_t *)data;

#if ENABLE_DEBUG
    char addr_str[KNX_ADDR_MAX_STR_LEN];

    DEBUG("[gnrc_knx_l7] _addr_write: addr=%s\n", knx_addr_physical_to_str(addr_str, addr, KNX_ADDR_MAX_STR_LEN));
#endif

    if (!device->info->programming_mode) {
        DEBUG("[gnrc_knx_l7] _addr_write: not in programming mode\n");
        return NULL;
    }

    /* delegate to implementation */
    if (device->event_callback != NULL) {
        knx_event_set_address_t event = {
            .address = addr
        };

        device->event_callback(device, KNX_DEVICE_EVENT_SET_ADDRESS, &event);
    }

    return NULL;
}

static gnrc_pktsnip_t *_mask_version_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 1) {
        DEBUG("[gnrc_knx_l7_apci] _addr_write: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, true);

    uint8_t descriptor = data[0] & 0x3f;

    DEBUG("[gnrc_knx_l7] _mask_version_read: descriptor=%d\n", descriptor);

    /* prepare response */
    uint8_t buf[] = { 0x00, 0x00, 0x00 };

    switch (descriptor) {
        case 0:
            buf[1] = (device->mask_version >> 8) & 0xFF;
            buf[2] = (device->mask_version) & 0xFF;
            break;
        default:
            DEBUG("[gnrc_knx_l7] _mask_version_read: unhandled\n");
            return NULL;
    }

    return _send_response(pkt, KNX_APCI_MASK_VERSION_RESPONSE, buf, 3);
}

static gnrc_pktsnip_t *_restart(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 1) {
        DEBUG("[gnrc_knx_l7_apci] _adc_read: incorrect payload length\n");
        return NULL;
    }

    DEBUG("[gnrc_knx_l7] _restart: restart request\n");

    /* delegate to implementation */
    if (device->event_callback != NULL) {
        device->event_callback(device, KNX_DEVICE_EVENT_RESTART, NULL);
    }

    return NULL;
}

static gnrc_pktsnip_t *_adc_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    (void)device;

    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 2) {
        DEBUG("[gnrc_knx_l7_apci] _adc_read: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, true);

    uint8_t channel = data[0] & 0x3f;
    uint8_t samples = data[1];

    DEBUG("[gnrc_knx_l7] _adc_read: channel=%d samples=%d\n", channel, samples);

    /* prepare response */
    uint8_t buf[4];

    buf[0] = channel;
    buf[1] = samples;
    buf[2] = 0;
    buf[3] = 0;

    return _send_response(pkt, KNX_APCI_ADC_RESPONSE, buf, 4);
}

static gnrc_pktsnip_t *_memory_read(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) != 3) {
        DEBUG("[gnrc_knx_l7_apci] _memory_read: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, true);

    uint8_t count = data[0] & 0x0f;
    uint16_t address = ntohs(*((uint16_t *)&(data[1])));

    DEBUG("[gnrc_knx_l7] _memory_read: addr=%04x count=%d \n", address, count);

    /* find the memory segment */
    knx_memory_segment_t *segment = knx_memory_find(device->segments, address, count);

    if (segment == NULL) {
        DEBUG("[gnrc_knx_l7] _memory_read: unmapped segment\n");
        return NULL;
    }

    /* read from the memory segment */
    uint8_t buf[16];

    buf[0] = data[0];
    buf[1] = data[1];
    buf[2] = data[2];

    knx_memory_read(segment, address, count, &buf[3]);

    return _send_response(pkt, KNX_APCI_MEMORY_RESPONSE, buf, 3 + count);
}

static gnrc_pktsnip_t *_memory_write(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_telegram_t *telegram = pkt->data;

    /* validate request */
    if (knx_telegram_get_payload_length(telegram) < 3) {
        DEBUG("[gnrc_knx_l7_apci] _memory_write: incorrect payload length\n");
        return NULL;
    }

    /* parse request */
    uint8_t *data = knx_telegram_get_payload(telegram, true);

    uint8_t count = data[0] & 0x0f;
    uint16_t address = ntohs(*((uint16_t *)&(data[1])));

    DEBUG("[gnrc_knx_l7] _memory_write: addr=%04x count=%d\n", address, count);

    /* find the memory segment */
    knx_memory_segment_t *segment = knx_memory_find(device->segments, address, count);

    if (segment == NULL) {
        DEBUG("[gnrc_knx_l7] _memory_write: unmapped segment\n");
        return NULL;
    }

    ssize_t res = knx_memory_write(segment, address, count, &data[3]);

    if (res < 0) {
        DEBUG("[gnrc_knx_l7] _memory_write: unable to write (%d)\n", (int)res);
        return NULL;
    }

    /* delegate memory write event to user */
    if (device->event_callback != NULL) {
        knx_event_mem_write_t event = {
            .segment = segment
        };

        device->event_callback(device, KNX_DEVICE_EVENT_MEM_WRITE, &event);
    }

    /* stop here if memory verification is disabled, otherwise reply with a
       memory read */
    if ((device->info->device_control & 0x04) == 0) {
        DEBUG("[gnrc_knx_l7] _memory_write: verification disabled\n");
        return NULL;
    }

    /* read from the memory segment */
    uint8_t buf[16];

    buf[0] = data[0];
    buf[1] = data[1];
    buf[2] = data[2];

    knx_memory_read(segment, address, count, &buf[3]);

    return _send_response(pkt, KNX_APCI_MEMORY_RESPONSE, buf, 3 + count);
}

static void _group_value_read(knx_device_t *device, gnrc_pktsnip_t *pkt, knx_com_object_t *com_object)
{
    (void)device;

    /* check if read is enabled */
    if ((com_object->access & KNX_COM_OBJECT_ACCESS_READ) == 0) {
        DEBUG("[gnrc_knx_l7] _group_value_read: read disabled\n");
        return;
    }

    uint8_t buf[15];
    uint8_t size;

    if (knx_com_object_size(com_object) < 1) {
        knx_com_object_read(com_object, &(buf[0]), 15);
        size = 0;
    }
    else {
        size = knx_com_object_read(com_object, &(buf[1]), 14);
    }

    /* multiple packets will be send, so perform here */
    gnrc_pktsnip_t *out = _send_response(pkt, KNX_APCI_GROUP_VALUE_RESPONSE, buf, size);

    if (out) {
        gnrc_netapi_send(gnrc_knx_l7_pid, out);
    }
}

static void _group_value_response(knx_device_t *device, gnrc_pktsnip_t *pkt, knx_com_object_t *com_object)
{
    knx_telegram_t *telegram = pkt->data;

    /* ensure telegram length is long enough */
    size_t actual = knx_telegram_get_payload_length(telegram);
    size_t expected = knx_com_object_size(com_object);

    if (actual != expected) {
        DEBUG("[gnrc_knx_l7] _group_value_response: expected %d bytes, got %d", (unsigned)expected, (unsigned)actual);
        return;
    }

    /* check if update is enabled */
    if ((com_object->access & KNX_COM_OBJECT_ACCESS_UPDATE) == 0) {
        DEBUG("[gnrc_knx_l7] _group_value_response: update disabled\n");
        return;
    }

    /* write value to com object */
    if (actual == 0) {
        uint8_t *data = knx_telegram_get_payload(telegram, true);
        uint8_t tmp = data[0] & 0x3f;

        knx_com_object_write(com_object, &tmp, expected);
    }
    else {
        knx_com_object_write(com_object, knx_telegram_get_payload(telegram, false), expected);
    }

    /* delegate update to user */
    if (device->event_callback != NULL) {
        knx_event_com_object_write_t event = {
            .com_object = com_object
        };

        device->event_callback(device, KNX_DEVICE_EVENT_COM_OBJECT_WRITE, &event);
    }
}

static void _group_value_write(knx_device_t *device, gnrc_pktsnip_t *pkt, knx_com_object_t *com_object)
{
    knx_telegram_t *telegram = pkt->data;

    /* ensure telegram length is long enough */
    size_t actual = knx_telegram_get_payload_length(telegram);
    size_t expected = knx_com_object_size(com_object) + 1;

    if (actual != expected) {
        DEBUG("[gnrc_knx_l7] _group_value_write: expected %d bytes, got %d\n", (unsigned)expected, (unsigned)actual);
        return;
    }

    /* check if write is enabled */
    if ((com_object->access & KNX_COM_OBJECT_ACCESS_WRITE) == 0) {
        DEBUG("[gnrc_knx_l7] _group_value_write: write disabled\n");
        return;
    }

    /* write value to com object */
    if (expected == 1) {
        uint8_t *data = knx_telegram_get_payload(telegram, true);
        uint8_t tmp = data[0] & 0x3f;

        knx_com_object_write(com_object, &tmp, expected);
    }
    else {
        knx_com_object_write(com_object, knx_telegram_get_payload(telegram, false), expected);
    }

    /* delegate update to user */
    if (device->event_callback != NULL) {
        knx_event_com_object_write_t event = {
            .com_object = com_object
        };

        device->event_callback(device, KNX_DEVICE_EVENT_COM_OBJECT_WRITE, &event);
    }
}

static gnrc_pktsnip_t *_group_value(knx_device_t *device, gnrc_pktsnip_t *pkt, next_cb_t *forward)
{
    knx_telegram_t *telegram = pkt->data;
    knx_addr_group_t addr;

    knx_telegram_get_dst_addr(telegram, &addr);

#if ENABLE_DEBUG
    char addr_str[KNX_ADDR_MAX_STR_LEN];

    DEBUG("[gnrc_knx_l7] _group_value: group address=%s\n", knx_addr_group_to_str(addr_str, &addr, sizeof(addr_str)));
#endif

    /* iterate over all associations for a given group address */
    knx_assoc_mapping_t *next = knx_assoc_iter_by_group_address(device->associations, NULL, &addr);

    if (next == NULL) {
        DEBUG("[gnrc_knx_l7] _group_value: no associations\n");
        return NULL;
    }

    while (next != NULL) {
        DEBUG("[gnrc_knx_l7] _group_value: group object access=%02x\n", next->com_object->access);

        /* skip disabled objects */
        if (next->com_object->access & KNX_COM_OBJECT_ACCESS_ENABLED) {
            forward(device, pkt, next->com_object);
        }
        else {
            DEBUG("[gnrc_knx_l7] _group_value: access disabled\n");
        }

        /* advance to next */
        next = knx_assoc_iter_by_group_address(device->associations, next, &addr);
    }

    return NULL;
}

gnrc_pktsnip_t *_handle_apci(knx_device_t *device, gnrc_pktsnip_t *pkt)
{
    knx_apci_t apci = knx_apci_get(pkt->data);

    DEBUG("[gnrc_knx_l7] _apci: type %02x\n", apci);

    switch (apci) {
        case KNX_APCI_GROUP_VALUE_READ:
            return _group_value(device, pkt, _group_value_read);
        case KNX_APCI_GROUP_VALUE_RESPONSE:
            return _group_value(device, pkt, _group_value_response);
        case KNX_APCI_GROUP_VALUE_WRITE:
            return _group_value(device, pkt, _group_value_write);
        case KNX_APCI_INDIVIDUAL_ADDR_WRITE:
            return _addr_write(device, pkt);
        case KNX_APCI_INDIVIDUAL_ADDR_READ:
            return _addr_read(device, pkt);
        // case KNX_APCI_INDIVIDUAL_ADDR_RESPONSE:
        //    break;
        case KNX_APCI_ADC_READ:
            return _adc_read(device, pkt);
        // case KNX_APCI_ADC_RESPONSE:
        //    break;
        case KNX_APCI_MEMORY_READ:
            return _memory_read(device, pkt);
        // case KNX_APCI_MEMORY_RESPONSE:
        //    break;
        case KNX_APCI_MEMORY_WRITE:
            return _memory_write(device, pkt);
        // case KNX_APCI_USER_MESSAGE:
        //    break;
        case KNX_APCI_MASK_VERSION_READ:
            return _mask_version_read(device, pkt);
        // case KNX_APCI_MASK_VERSION_RESPONSE:
        //    break;
        case KNX_APCI_RESTART:
            return _restart(device, pkt);
        case KNX_APCI_ESCAPE:
            return _handle_apci_extended(device, pkt);
        default:
            DEBUG("[gnrc_knx_l7_apci] _apci: unhandled\n");
    }

    return NULL;
}

static void _receive(gnrc_pktsnip_t *pkt)
{
    /* drop telegrams of wrong type */
    if (pkt->type != GNRC_NETTYPE_KNX_L7) {
        DEBUG("[gnrc_knx_l7] _receive: telegram not an application layer telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* drop telegrams that are not telegrams */
    if (!knx_telegram_is(pkt->data, pkt->size)) {
        DEBUG("[gnrc_knx_l7] _receive: invalid or incomplete telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* handle services */
    gnrc_pktsnip_t *out = _handle_apci(device, pkt);

    if (out) {
        gnrc_netapi_send(gnrc_knx_l7_pid, out);
    }

    /* packet can be released */
    gnrc_pktbuf_release(pkt);
}

static void _send(gnrc_pktsnip_t *pkt)
{
    pkt->type = GNRC_NETTYPE_KNX_L4;

    /* forward packet to the network layer */
    if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_KNX_L4, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("[gnrc_knx_l7] send: cannot send due to missing transport layer\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void *_event_loop(void *args)
{
    (void)args;

    msg_t msg, reply, msg_q[GNRC_KNX_L7_MSG_QUEUE_SIZE];
    gnrc_netreg_entry_t me_reg = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                            sched_active_pid);

    msg_init_queue(msg_q, GNRC_KNX_L7_MSG_QUEUE_SIZE);

    /* register interest in all KNX packets */
    gnrc_netreg_register(GNRC_NETTYPE_KNX_L7, &me_reg);

    /* preinitialize ACK */
    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
    reply.content.value = (uint32_t)-ENOTSUP;

    /* start event loop */
    while (1) {
        // DEBUG("[gnrc_knx_l7] _event_loop: waiting for incoming message.\n");
        msg_receive(&msg);

        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                DEBUG("[gnrc_knx_l7] _event_loop: GNRC_NETAPI_MSG_TYPE_RCV received\n");
                _receive(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("[gnrc_knx_l7] _event_loop: GNRC_NETAPI_MSG_TYPE_SND received\n");
                _send(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
                DEBUG("[gnrc_knx_l7] _event_loop: reply to unsupported get/set\n");
                msg_reply(&msg, &reply);
                break;
            default:
                DEBUG("[gnrc_knx_l7] _event_loop: received unidentified message\n");
                break;
        }
    }

    return NULL;
}

void gnrc_knx_l7_update_com_object(knx_device_t *device_, knx_com_object_t *com_object)
{
    size_t size = knx_com_object_size(com_object) + 1;

    /* allocate output telegram */
    gnrc_pktsnip_t *out = gnrc_pktbuf_add(NULL, NULL, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN + size, GNRC_NETTYPE_KNX_L7);

    if (out == NULL) {
        DEBUG("[gnrc_knx_l7] gnrc_knx_l7_update_com_object: unable to allocate telegram\n");
        return;
    }

    /* prepare data */
    knx_telegram_set_type(out->data, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_group_addressed(out->data, true);
    knx_telegram_set_src_addr(out->data, &(device_->address));
    knx_telegram_set_payload_length(out->data, size);
    knx_telegram_set_routing_count(out->data, 6);
    knx_telegram_set_repeated(out->data, false);
    knx_telegram_set_priority(out->data, com_object->priority);

    knx_tpci_set(out->data, KNX_TPCI_UDP);

    if (size >= 2) {
        knx_com_object_read(com_object, &((uint8_t *)out->data)[8], size);
    }
    else {
        knx_com_object_read(com_object, &((uint8_t *)out->data)[7], size);
    }

    knx_apci_set(out->data, KNX_APCI_GROUP_VALUE_WRITE);

    /* multiple group addresses can be associated with a communication object,
       so loop over all associations and send a telegram for each involved
       association */
    knx_assoc_mapping_t *next = knx_assoc_iter_by_com_object(device_->associations, NULL, com_object);

    if (next == NULL) {
        DEBUG("[gnrc_knx_l7] gnrc_knx_l7_update_com_object: no associations\n");
        gnrc_pktbuf_release(out);
        return;
    }

    while (next != NULL) {
        knx_telegram_set_dst_addr(out->data, &(next->group_addr));

        /* send telegram */
        gnrc_pktbuf_hold(out, 1);

        if (gnrc_netapi_send(gnrc_knx_l7_pid, out) != 1) {
            DEBUG("[gnrc_knx_l7] gnrc_knx_l7_update_com_object: unable to send telegram to gnrc_knx_l7\n");
            gnrc_pktbuf_release(out);
            return;
        }

        /* advance to next */
        next = knx_assoc_iter_by_com_object(device_->associations, next, com_object);
    }

    /* we are done with the package */
    gnrc_pktbuf_release(out);
}

kernel_pid_t gnrc_knx_l7_init(knx_device_t *device_)
{
    if (gnrc_knx_l7_pid == KERNEL_PID_UNDEF) {
        gnrc_knx_l7_pid = thread_create(_stack, sizeof(_stack), GNRC_KNX_L7_PRIO,
                                        THREAD_CREATE_STACKTEST,
                                        _event_loop, NULL, "knx_l7");

        /* store device parameters */
        device = device_;
    }

    return gnrc_knx_l7_pid;
}
