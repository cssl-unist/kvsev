#ifndef _SEV_H_
#define _SEV_H_

#include <stdint.h>
#include <linux/kvm.h>
#include <linux/psp-sev.h>

#define SEV_DEV_PATH	"/dev/sev"
#define KVM_DEV_PATH	"/dev/kvm"

/* custom SEV ioctl's from sev_helper */
#define SEV_ATTEST		_IOWR('a', 10, uint64_t)
#define SEV_DEACTIVATE	_IOWR('a', 11, uint64_t)
#define SEV_WBINVD		_IOWR('a', 12, uint64_t)

/* SEV ioctl request struct */
struct sev_helper_deactivate_cmd {
	__u32	handle;
};

int open_path_or_exit(const char *path, int flags);

/* SEV related functions */
void sev_ioctl(int vmfd, int cmd_id, void *data);
int sev_vm_init(int vmfd);
int sev_vm_create(int vmfd, int handle);
void sev_vm_update(int vmfd, uint8_t *mem, int len);
void sev_vm_measure(int vmfd, void *buf, int len);
void sev_vm_finish(int vmfd);
void sev_vm_status(int vmfd);

/* KVM related functions */
void kvm_vm_init(int *kvm, int *vmfd);
// void kvm_vm_setup(int kvm, int vmfd, int vcpufd, void *mem);

#endif /* _SEV_H_ */
