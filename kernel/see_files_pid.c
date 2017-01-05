#define _POSIX_SOURCE

#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

struct see_files_struct {
  int files_open;
  long unsigned int *n_nodes;
};

asmlinkage long sys_see_files_pid(int pid, struct see_files_struct *pointer) {
  struct task_struct *process;

  // buscando processo
  process = find_task_by_vpid(pid);

  if (process > 0) { // pid foi encontrado
    struct files_struct *files;
    struct fdtable *files_table;
    long unsigned int *kn_nodes;

    int num_fds = 3, i = 0; // os 3 primeiros FD são reservados para informacoes do processo

    // pegando a file struct da task struct
    files = process->files;

    // recuperando os FDs
    files_table = files_fdtable(files);

    // iterando pelos os FDs, para descobri quantos fds existem
    while(files_table->fd[num_fds] != NULL) {
      num_fds++;
    }

    // adicionando ao ponteiro de referencia a quantidade de arquivos do processo
    pointer->files_open = num_fds - 3;
    printk(KERN_DEBUG "---> num fds %d\n", pointer->files_open);

    // criando um array de long onde o tamanho é a quantidade de arquivos do processo
    // o array está alocando no kernel lvl
    kn_nodes = (void *)kmalloc(pointer->files_open * sizeof(long unsigned int), GFP_USER);

    // loop na quantidade de arquivos
    while(i < pointer->files_open) {
      // acessando o inode e pegando o seu numero
      kn_nodes[i] = (long unsigned int) files_table->fd[i + 3]->f_inode->i_ino;

      // printando o numero do inode
      printk(KERN_DEBUG "---> num %d inode %lu\n", i, kn_nodes[i]);
      i++;
    }

    // copiando array de numeros de inode do kernel lvl para ponteiro de referencia que está no 
    // user lvl
    copy_to_user(pointer->n_nodes, kn_nodes, pointer->files_open * sizeof(long unsigned int));

    return 1;
  }

  return 0;
}

