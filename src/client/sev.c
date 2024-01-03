#include "sev.h"
#include "util.h"
#include <errno.h>

int open_path_or_exit(const char *path, int flags) {
	int fd;
	fd = open(path, flags);
	if (fd < 0) {
		printf("%s not available (errno: %d)\n", path, errno);
		exit(1);
	}
	return fd;
}

int open_sev_dev_path_or_exit(void) {
	return open_path_or_exit(SEV_DEV_PATH, 0);
}

/* SEV related functions */

/*
 * Function: sev_ioctl
 * Description:
 *	Sends KVM supported SEV ioctls to KVM driver
 * Params:
 *	@vmfd: 		virtual machine file descriptor
 *	@cmd_id:	SEV command id
 *	@data:		SEV command data buffer address
 *	@sevfd:		SEV driver file descriptor
 * Returns:
 *	
 */
void sev_ioctl(int vmfd, int cmd_id, void *data) {

	int ret, fd;
	
	fd = open_sev_dev_path_or_exit();

	struct kvm_sev_cmd cmd = {
		.id 	= cmd_id,
		.data 	= (uint64_t)data,
		.sev_fd = fd,
	};
	ret = ioctl(vmfd, KVM_MEMORY_ENCRYPT_OP, &cmd);
	if (cmd.error != SEV_RET_SUCCESS) {
		printf("%d failed: return code: %d, errno: %d, fw error: %d\n",
				cmd_id,
				ret,
				errno,
				cmd.error);
		exit(1);
	}
	close(fd);
	return;
}

/*
 * Function: sev_vm_init
 * Description:
 *	Send SEV_INIT ioctl
 * Params:
 *	@vmfd:		VM file descriptor
 */
int sev_vm_init(int vmfd) {
	int ret;
	sev_ioctl(vmfd, KVM_SEV_INIT, NULL);
#if 0
	ret = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);
	if (ret < 0) {
		printf("KVM_CREATE_VCPU\n");
		exit(1);
	}
#endif
	return ret;
}

/*
 * Function: sev_vm_create
 * Description:
 *	Sends SEV_LAUNCH_START request to fw
 * Params:
 *	@vmfd:		VM file descriptor
 *	@handle:	handle of VM (in case it's not 0)
 * Return:
 *	New SEV VM handle assigned by the firmware
 */
int sev_vm_create(int vmfd, int handle) {
	struct kvm_sev_launch_start start = { 0 };
	sev_ioctl(vmfd, KVM_SEV_LAUNCH_START, &start);
	return start.handle;
}

/*
 * Function: sev_vm_update
 * Description:
 *	Sends SEV_LAUNCH_UPDATE_DATA request to fw
 * Params:
 *	@vmfd:		VM file descriptor
 *	@mem:		Memory address of the code/data to be updated
 *	@len:		Length of the updated code/data
 * Note:
 *	Memory address [mem, mem + len) should be encrypted after this
 */
void sev_vm_update(int vmfd, uint8_t *mem, int len) {
	struct kvm_sev_launch_update_data update = { 0 };
	update.uaddr 	= (uint64_t)mem;
	update.len		= len;
	sev_ioctl(vmfd, KVM_SEV_LAUNCH_UPDATE_DATA, &update);

	return;
}

/*
 * Function: sev_vm_measure
 * Description:
 *	Sends SEV_LAUNCH_MEASURE request to fw
 * Params:
 *	@vmfd:		VM file descriptor
 *	@buf:		Memory address of the buffer to contain the masurement
 *	@len:		Length of the memory buffer
 */
void sev_vm_measure(int vmfd, void *buf, int len) {
	struct kvm_sev_launch_measure measure = { 0 };
	measure.uaddr 	= (uint64_t)buf;
	measure.len 	= len;
	sev_ioctl(vmfd, KVM_SEV_LAUNCH_MEASURE, &measure);
	return;
}

/*
 * Function: sev_vm_finish
 * Description:
 *	Sends SEV_LAUNCH_FINISH request to fw
 * Params:
 *	@vmfd:		VM file descriptor
 */
void sev_vm_finish(int vmfd) {
	sev_ioctl(vmfd, KVM_SEV_LAUNCH_FINISH, NULL);
	return;
}

/*
 * Function: sev_vm_status
 * Description:
 *	Prints SEV VM handle and state for debugging purposes
 * Params:
 *	@vmfd:		VM file descriptor
 */
void sev_vm_status(int vmfd) {
	struct kvm_sev_guest_status status;
	sev_ioctl(vmfd, KVM_SEV_GUEST_STATUS, &status);
	printf("handle: %d\n", status.handle);
	printf("status: %d\n", status.state);
}

/* KVM related functions */
void kvm_vm_init(int *kvm, int *vmfd) {
	int kvm_tmp, vmfd_tmp;
	kvm_tmp = open(KVM_DEV_PATH, O_RDWR | O_CLOEXEC);
	if (kvm_tmp < 0) {
		fprintf(stdout, "[SEVault] failed to open KVM dev\n");
		exit(1);
	}
	vmfd_tmp = ioctl(kvm_tmp, KVM_CREATE_VM, (unsigned long)0);
	if (vmfd_tmp == -1) {
		fprintf(stdout, "[SEVault] failed to KVM_CREATE_VM\n");
		exit(1);
	}

	*kvm = kvm_tmp;
	*vmfd = vmfd_tmp;

	close(kvm_tmp);
	return;
}

static void kvm_vm_setup(int kvm, int vmfd, int vcpufd, void *mem) {

}
