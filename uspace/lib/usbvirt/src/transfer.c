/*
 * Copyright (c) 2011 Vojtech Horky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup libusbvirt
 * @{
 */
/** @file
 *
 */
#include <usbvirt/device.h>
#include <usb/debug.h>
#include <errno.h>
#include <assert.h>
#include "private.h"

static int usbvirt_control_transfer(usbvirt_device_t *dev,
    void *setup, size_t setup_size,
    void *data, size_t data_size, size_t *data_size_sent)
{
	assert(dev);
	assert(dev->ops);

	if (setup_size != sizeof(usb_device_request_setup_packet_t)) {
		return ESTALL;
	}
	usb_device_request_setup_packet_t *setup_packet = setup;
	if (data_size != setup_packet->length) {
		return ESTALL;
	}

	int rc;

	/* Run user handler first. */
	rc = process_control_transfer(dev, dev->ops->control,
	    setup_packet, data, data_size_sent);
	if (rc != EFORWARD) {
		return rc;
	}

	/* Run the library handlers afterwards. */
	rc = process_control_transfer(dev, library_handlers,
	    setup_packet, data, data_size_sent);

	if (rc == EFORWARD) {
		usb_log_warning("Control transfer {%s} not handled.\n",
		    usb_debug_str_buffer(setup, setup_size, 10));
		rc = EBADCHECKSUM;
	}

	return rc;
}

int usbvirt_control_write(usbvirt_device_t *dev, void *setup, size_t setup_size,
    void *data, size_t data_size)
{
	return usbvirt_control_transfer(dev, setup, setup_size,
	    data, data_size, NULL);
}

int usbvirt_control_read(usbvirt_device_t *dev, void *setup, size_t setup_size,
    void *data, size_t data_size, size_t *data_size_sent)
{
	return usbvirt_control_transfer(dev, setup, setup_size,
	    data, data_size, data_size_sent);
}

int usbvirt_data_out(usbvirt_device_t *dev, usb_transfer_type_t transf_type,
    usb_endpoint_t endpoint, void *data, size_t data_size)
{
	if ((endpoint <= 0) || (endpoint >= USBVIRT_ENDPOINT_MAX)) {
		return ERANGE;
	}
	if ((dev->ops == NULL) || (dev->ops->data_out[endpoint] == NULL)) {
		return ENOTSUP;
	}

	int rc = dev->ops->data_out[endpoint](dev, endpoint, transf_type,
	    data, data_size);

	return rc;
}

int usbvirt_data_in(usbvirt_device_t *dev, usb_transfer_type_t transf_type,
    usb_endpoint_t endpoint, void *data, size_t data_size, size_t *data_size_sent)
{
	if ((endpoint <= 0) || (endpoint >= USBVIRT_ENDPOINT_MAX)) {
		return ERANGE;
	}
	if ((dev->ops == NULL) || (dev->ops->data_in[endpoint] == NULL)) {
		return ENOTSUP;
	}

	size_t data_size_sent_tmp;
	int rc = dev->ops->data_in[endpoint](dev, endpoint, transf_type,
	    data, data_size, &data_size_sent_tmp);

	if (rc != EOK) {
		return rc;
	}

	if (data_size_sent != NULL) {
		*data_size_sent = data_size_sent_tmp;
	}

	return EOK;
}

/**
 * @}
 */
