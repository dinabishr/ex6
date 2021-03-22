#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/moduleparam.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<asm/segment.h>
#include<linux/buffer_head.h>
#include<linux/processor.h>
#include<linux/kallsyms.h>
#include<linux/cred.h>


MODULE_LICENSE("GPL");
int forkcount=0;
typedef asmlinkage long(*sys_call_ptr_t)(const struct pt_regs *);
static sys_call_ptr_t *sys_ptr;
static sys_call_ptr_t old_fork;
void **ptr;
struct myfile{

	struct file *f;
	mm_segment_t fs;
	loff_t pos ;

};


struct myfile *open_file_for_read(char *filename){
	struct myfile *m_f=kmalloc(sizeof(struct myfile),GFP_KERNEL);
	m_f->fs = get_fs();
	m_f->pos =14;
	m_f->f=filp_open(filename,O_RDONLY,0);
return m_f;

}

volatile int read_from_file_until(struct myfile *mf,char *buff,unsigned long vlen,char c){
	int result;

	set_fs(get_ds());
	result=vfs_read(mf->f,buff,vlen,&(mf-> pos));
	set_fs(mf->fs);


return result;

}
volatile int read_from_sys_file(struct myfile *mf,char *buff,unsigned long vlen,char c){
	int result;
	set_fs(get_ds());
	result=vfs_read(mf->f,buff,vlen,&(mf-> pos));
	
	mf->pos -=vlen;
	mf->pos +=strchr(buff,c)- buff+1;
	buff[strchr (buff,c) - buff] =0;
	set_fs(mf->fs);
return result;

}

void close_file(struct myfile *mf){

	filp_close(mf->f,NULL);
}

static asmlinkage long myhook(const struct pt_regs *regs){
	forkcount++;
	printk(KERN_INFO "%d\n",forkcount);


	if(forkcount%10 ==0){
		printk(KERN_INFO "%d\n",forkcount);

	}	
	/*clone();*/
	/*old_fork(regs);*/
	return old_fork(regs);

}

static int init(void){
	struct myfile *_file;
	struct myfile *s_file;
	char *buf=kmalloc(80,GFP_KERNEL);
	char *sysbuf=kmalloc(300,GFP_KERNEL);
	char *addr=kmalloc(16,GFP_KERNEL);
	int val,r,i;
	char *filename=kmalloc(34,GFP_KERNEL);
	long long p;
/*	void **ptr;*/
	memset(sysbuf,0,300);
	memset(addr,0,16);
	for(i=0;i<34;i++){filename[i]=0;};
	for(i=0;i<80;i++){buf[i]=0;};
/*	filename="/boot/System.map-4.19.0-13-amd64";*/
	strncat(filename,"/boot/System.map-",17);
	addr[16]=NULL;
	printk(KERN_INFO "Hello World CSCE-3402 :)\n");
	_file = open_file_for_read("/proc/version");
	val=read_from_file_until(_file,buf,16,'.');
	printk(KERN_INFO "%s\n",buf);
	strncat(filename,buf,16);
	filename[32]=NULL;
	printk(KERN_INFO "%s\n",filename);
	
	s_file= open_file_for_read(filename);
	while(strstr(sysbuf,"sys_call_table")==NULL){
	 r=read_from_sys_file(s_file,sysbuf,300,'\n');
	 }
	 printk(KERN_INFO "%s\n",sysbuf);
	 strncat(addr,sysbuf,16);
	 printk(KERN_INFO "%s\n",addr);
	 sscanf(addr,"%llx",&p);
	 ptr=p;
	/* sys_ptr=(sys_call_ptr_t *)kallsyms_lookup_name("sys_call_table");*/
	 printk(KERN_INFO "%p\n",ptr[__NR_clone]);
	 old_fork=ptr[__NR_clone];
	 write_cr0(read_cr0() & (~0x10000));
	ptr[__NR_clone]=(sys_call_ptr_t)myhook;
	 printk(KERN_INFO "%p\n",ptr[__NR_clone]);
	 
	write_cr0(read_cr0()|0x10000);


	close_file(s_file);
	close_file(_file);
	kfree(buf);
	kfree(sysbuf);
	kfree(addr);
	return 0;
}



static void cleanup(void){

	printk(KERN_INFO "Bye bye CSCE-3402 :) \n");
	write_cr0(read_cr0() & (~0x10000));
	ptr[__NR_clone]=old_fork;
	write_cr0(read_cr0()|0x10000);

}


module_init(init);
module_exit(cleanup);




