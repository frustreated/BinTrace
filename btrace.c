/* Mohammed Ghannam 0x36 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/ptrace.h>

#include "utils.h"
#include "proc.h"


const struct option lo[]=
  {
    {"attach",required_argument,0,'p'},
    {"address" , required_argument,0,'a'},
    {"offset",required_argument,0,'o'},
    {"dump",no_argument,0,'d'},
    {"raw",no_argument,0,'r'},
    {"data",no_argument,0,'p'},
    {"target",required_argument,0,'t'},
    {"args",required_argument,0,'A'},
    {NULL,0,NULL,0}
  };

struct bt_opts
{
  u_int8_t  pid_opt;
  u_int8_t force_addr_opt;
  u_int8_t off_opt;
  u_int8_t raw_opt;
  u_int8_t use_data_opt;
  u_int8_t target_opt;
  u_int8_t target_has_args;

};


static struct btproc *parse_args(int,char**,struct bt_opts *);
static void btrace_banner(char *,int );

int main(int argc,char **argv)
{
  struct bt_opts opts;
  struct btproc *bt_proc;
  struct perms target_perms;
  
  bt_proc = parse_args(argc,argv,&opts);

  if (opts.target_opt && opts.pid_opt)
    {
      bt_proc_destroy(bt_proc);
      printfd(2,FATAL" You can't choose target and pid together !\n");
      btrace_banner(*argv,1);
    }else
    {
      /* using target executable */
      if(opts.target_opt)
	{
	  bt_proc->exec = check_target_path(bt_proc->pi->pi_target,&target_perms);
	  
	  if(!bt_proc->exec)
	    btrace_banner(*argv,1);
	  
	  if(opts.target_has_args)
	    bt_proc->args_parser(bt_proc->pi->pi_args,bt_proc);
	  else
	    bt_proc->proc_arguments[0] = strdup(bt_proc->exec);
	      
	  
	  if((!opts.force_addr_opt && opts.off_opt) ||
	     (opts.force_addr_opt && !opts.off_opt))
	    {
	      printfd(STDERR_FILENO,WARN"You may choose both of address and offset !\n");
	      bt_proc_destroy(bt_proc);
	      btrace_banner(*argv,1);
	    }
	  else
	    {
	      bt_proc->pi->pi_map[0]= bt_proc->pi->pi_address;
	      bt_proc->pi->pi_map[1]= bt_proc->pi->pi_address+bt_proc->pi->pi_offset;
	    }
	  
	  
	  exec_target(bt_proc);
	  
	  //dump_using_memory(bt_proc);
	}
      
    }/* end of using executable target */
    
}


static struct btproc *parse_args(int argc,char **argv,struct bt_opts *opts)
{
  int opt,long_opt_index=0;
  char *target_args;
  struct procinfo *pi;
  struct btproc *bt;
  
  int i;

  /* initialize our options structure  */
  opts->pid_opt=0;
  opts->force_addr_opt =0;
  opts->off_opt  =0;
  opts->raw_opt  =0;
  opts->target_opt=0;
  opts->target_has_args=0;
  /* Default dump */
  opts->use_data_opt=1;
  opts->off_opt=0;
  bt = bt_proc_init();
  pi = bt->pi;
  
  while( (opt = getopt_long(argc,argv,"a:p:o:drgt:A:h",lo,&long_opt_index))!=-1)
    {
      switch(opt)
	{
	case 'a':
	  pi->pi_address = strtol(optarg,NULL,0x10);
	  opts->force_addr_opt |=1;
	  break;
	case 'p':
	  pi->pi_pid = atoi(optarg); /* get pid from user */
	  opts->pid_opt |=1;
	  break;
	case 't':
	  pi->pi_target = strdup(optarg);
	  opts->target_opt|=1;
	  break;
	case 'A':
	  pi->pi_args = strdup(optarg);
	  opts->target_has_args |=1;
	  break;
	case 'r':
	  opts->raw_opt |=1;
	  opts->use_data_opt &=0; /* disable data representation */
	  break;
	case 'h':
	  btrace_banner(*argv,0);
	  break;
	case 'o':
	  pi->pi_offset = strtol(optarg,NULL,10);
	  opts->off_opt|=1;
	  break;
	case 'd':
	  opts->use_data_opt |=1;
	  break;
	default:
	  btrace_banner(*argv,-1);
	}
    }
  return bt;
}

void btrace_banner(char *arg,int status)
{
  //printfd(1,"+-----------------------------------------+\n");
  printfd(1,"Usage : %s <options>  \n",arg);
  printfd(1,"Written By  M.Ghannam (Simo36)\n");
  printfd(1,
	  "  -p  --attach <process id>  \t set a process id\n"
	  "  -a --address <memory_addr> \t memory address\n"
	  "  -o --offset  <byte offset> \t how many byte you want to leak?\n"
	  "  -t --target  <binary process>\t binary process wich you want to leak from\n"
	  "  -A --args    [bianry args] \t target arguments (optional)\n"
	  "  -d --dump                  \t dump memory data\n"
	  "  -r --raw                   \t dump raw data \n"
	  
	  );
  exit(status);
}

