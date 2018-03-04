#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/syscalls.h>

/*
 * system call to print the start and end addresses of the code, data, and heap segments of a process
 * <TEAM09/>
 */

SYSCALL_DEFINE1(show_vm_areas, pid_t, pid){
	struct task_struct* t;
	pid_t tmp_pid;
    struct vm_area_struct *current_vma;
    char locked[6];
    char size_in_bytes[17];
	unsigned long vm_start;		/* Our start address within vm_mm. */
	unsigned long vm_end;		/* The first byte after our end address */
	int present_page_no;
	unsigned long i;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;
	spinlock_t *ptl;

	if(pid<0)
		return -1;

	//check for pid=0
	tmp_pid = pid;
	if(pid == 0)
		tmp_pid = current->pid;

	t = find_task_by_pid_ns(tmp_pid, &init_pid_ns);
	if(t == NULL)
		return -1;

	current_vma = t->mm->mmap;
    printk(KERN_INFO "<TEAM09>: [Memory-mapped areas of process %d]\n",(int) tmp_pid);


	while(current_vma != NULL){
	    strcpy(locked, ",");
	    if(current_vma->vm_flags & VM_LOCKED)	//VM_LOCKED = 0x00002000
	        strcpy(locked, " [L],");

	    present_page_no = 0;
	    vm_start = current_vma->vm_start;
	    vm_end = current_vma->vm_end;
	    /* for each vma we iterate from its beginning to its end and check
	     * if each page is present in memory or not
	     */
	    for(i=vm_start; i<vm_end; i+=PAGE_SIZE){

	    	pgd = pgd_offset(current_vma->vm_mm, vm_start);
	    	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
	    		return -1;
	    	/* We do not have pud in arm architecture, however this function converts
	    	 * pgd_t* to pmd_t*
	    	 */
	    	pud = pud_offset(pgd, vm_start);
	    	if (pud_none(*pud) || unlikely(pud_bad(*pud)))
	    		return -1;

	    	pmd = pmd_offset(pud, vm_start);
	    	if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
	    		return -1;

	    	ptep = pte_offset_map_lock(current_vma->vm_mm, pmd, vm_start, &ptl);
	    	if (!ptep)
	    		return -1;
	    	if (pte_present(*ptep))
	    		present_page_no++;

	    	pte_unmap_unlock(ptep, ptl);

	    }
	    sprintf(size_in_bytes, "%lu", (current_vma->vm_end - current_vma->vm_start));

	    printk(KERN_INFO "<TEAM09>: %08x - %08x: %-10s bytes%-06s %d pages in physical memory\n", (unsigned int)current_vma->vm_start, (unsigned int) current_vma->vm_end, size_in_bytes, locked, present_page_no);
	    current_vma = current_vma->vm_next;
	}
	return 0;
}
