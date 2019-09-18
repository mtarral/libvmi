/* The LibVMI Library is an introspection library that simplifies access to
 * memory in a target virtual machine or in a file containing a dump of
 * a system's physical memory.  LibVMI is based on the XenAccess Library.
 *
 * Author: Matthew Fusaro (matthew.fusaro@zentific.com)
 *
 * This file is part of LibVMI.
 *
 * LibVMI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * LibVMI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with LibVMI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <inttypes.h>
#include <signal.h>

#include <libvmi/libvmi.h>
#include <libvmi/events.h>

vmi_event_t single_event;

static int interrupted = 0;
static void close_handler(int sig)
{
    interrupted = sig;
}

event_response_t single_step_callback(vmi_instance_t vmi, vmi_event_t *event)
{
    vmi = vmi;
    printf("Single-step event: VCPU:%u  GFN %"PRIx64" GLA %016"PRIx64"\n",
           event->vcpu_id,
           event->ss_event.gfn,
           event->ss_event.gla);

    return 0;
}

int main (int argc, char **argv)
{
    vmi_instance_t vmi;
    vmi_init_data_t init_data = {0};
    vmi_mode_t mode;

    struct sigaction act;

    char *name = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: single_step_example <name of VM> [kvmi socket path]\n");
        exit(1);
    }

    // Arg 1 is the VM name.
    name = argv[1];

    /* for a clean exit */
    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGALRM, &act, NULL);

    if (argc == 3) {
        char *kvmi_socket = argv[2];

        // fill init_data
        init_data.count = 1;
        init_data.entry[0].type = VMI_INIT_DATA_KVMI_SOCKET;
        init_data.entry[0].data = kvmi_socket;
    }

    if (VMI_FAILURE == vmi_get_access_mode(NULL, (void*)name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, &init_data, &mode)) {
        fprintf(stderr, "Failed to get access mode\n");
        return 1;
    }

    /* initialize the libvmi library */
    if (VMI_FAILURE ==
        vmi_init(&vmi, mode, name, VMI_INIT_DOMAINNAME | VMI_INIT_EVENTS, &init_data, NULL)) {
        fprintf(stderr, "Failed to init LibVMI library.\n");
        return 1;
    }

    printf("LibVMI init succeeded!\n");

    //Single step setup
    memset(&single_event, 0, sizeof(vmi_event_t));
    single_event.version = VMI_EVENTS_VERSION;
    single_event.type = VMI_EVENT_SINGLESTEP;
    single_event.callback = single_step_callback;
    single_event.ss_event.enable = 1;
    SET_VCPU_SINGLESTEP(single_event.ss_event, 0);
    vmi_register_event(vmi, &single_event);
    while (!interrupted ) {
        printf("Waiting for events...\n");
        vmi_events_listen(vmi,500);
    }
    printf("Finished with test.\n");

    // cleanup any memory associated with the libvmi instance
    vmi_destroy(vmi);

    return 0;
}
