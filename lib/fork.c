// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");

	void *addr = (void*) (pn*PGSIZE);

	if ((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW)) {

		if ((r = sys_page_map(0, addr, envid, addr, PTE_U | PTE_P | PTE_COW))) {
			panic("duppage: %e", r);
                }

		if ((r = sys_page_map(0, addr, 0, addr, PTE_U | PTE_P | PTE_COW))) {
			panic("duppage: %e", r);
                }
	} else {
                sys_page_map(0, addr, envid, addr, PTE_U | PTE_P);
        }

	return 0;
}

static int
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int error;

       

        //direccion de I/O , hacer return, no hay que mapear de nuevo
        if ((perm & PTE_PCD) || (PTE_PWT & perm ) || (PTE_MAPPED & perm )) {     
                return 0;
        }

        perm = perm & PTE_SYSCALL;

        //si es escritura
	if ((perm & PTE_W)) {
                /*
		error = sys_page_alloc(dstenv, 0, perm);
		if (error) 
			panic("dup_or_share - sys_page_alloc: %e", error);
		error = sys_page_map(0, va, dstenv, 0, perm);
		if (error)
			panic("dup_or_share - sys_page_map with W perm: %e", error);	
                */
                
                //COPIA DEL DUPPAGE DE DUMBFORK

                error = sys_page_alloc(dstenv, va, perm);
		if (error) 
			panic("dup_or_share - sys_page_alloc: %e", error);
                
                //dir aux para la pag, dumbfork usaba UTEMP
                //pero en este fork tambien se copia esa pag, no se tendria que usar (creo)
                //uso la dir 0 (hay otra mejor?)
                int aux_addr = 0;

                error = sys_page_map(dstenv, va, 0, (void*) aux_addr, perm);
		if (error)
			panic("dup_or_share - sys_page_map with W perm: %e", error);


                memmove((void*) aux_addr, va, PGSIZE);
	        if ((error = sys_page_unmap(0, (void*) aux_addr)) < 0)
		        panic("dup_or_share - sys_page_unmap: %e", error);


	} else { //si es solo lectura
                error = sys_page_map(0, va, dstenv, va, perm);
		if (error) {
			panic("dup_or_share - sys_page_map without W perm: %e", error);		
		}
	}

	return 0;
}

envid_t
fork_v0(void)
{
	envid_t envid;
	uint8_t *addr;
	int r;

	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
        
        for (addr = 0; addr < (uint8_t*) UTOP; addr += PGSIZE) {
    	// Si la page directory y la page table son validas..

    	        if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P)
    			&& (uvpt[PGNUM(addr)] & PTE_U)) {
			dup_or_share(envid, addr, uvpt[PGNUM(addr)]);
    	        }
        }


        if (sys_env_set_status(envid, ENV_RUNNABLE)) {
                panic("fork: cannot set env status");
        }
    
        return envid;
}


//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
    return fork_v0();
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
