#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

/*
 *  �萔�̒�`
 */

#define BUFLEN    1024     /* �R�}���h�p�̃o�b�t�@�̑傫�� */
#define MAXARGNUM  256     /* �ő�̈����̐� */

/*
 *  ���[�J���v���g�^�C�v�錾
 */

char prompt[50]="Command";     /* �v�����v�g*/


typedef struct dir_list{
  char           name[100];
  struct dir_list *prev;
  struct dir_list *next;
} DIR_LIST;

typedef struct hist_list{
  char         name[100];
  struct hist_list *prev;
  struct hist_list *next;
}HIST_LIST;


typedef struct ail_list{
  char pre_name[100];
  char post_name[100];
  struct ail_list *prev;
  struct ail_list *next;
}AIL_LIST;

DIR_LIST *root=NULL; /* DIR_LIST�̐擪 */
HIST_LIST *head=NULL;/* HIST_LIST�̐擪 */
AIL_LIST  *lead=NULL;/* AIL_LIST�̐擪*/
int counter=0;       /*history_comannd�̃J�E���^*/
int t=0;             /*����֐��p�̃t���O*/

int parse(char [], char *[]);
void execute_command(char *[], int);
//����֐�
void cd_comannd(char *[]);
void pushd_comannd(void);
void dirs_comannd(DIR_LIST*);
void popd_comannd(void);
DIR_LIST *new_item( DIR_LIST *post_item, DIR_LIST *pre_item);
DIR_LIST *insert(DIR_LIST*);
void print_list(DIR_LIST *item);
void history_comannd(HIST_LIST*);
HIST_LIST *new_thing(char *[],HIST_LIST *post_thing,HIST_LIST *pre_thing);
HIST_LIST *extend(char *[],HIST_LIST *);
void rm_hist(void);
void ex_comannd(int);
void exstr_comannd(char *[],int);
void *asta_trans(int,char *[],int);
AIL_LIST *new_staff(char *[],AIL_LIST *post_staff,AIL_LIST *pre_staff);
void alias_comannd(char *[]);
void repla(char *[],AIL_LIST *);
void unalias_comannd(void);
/*----------------------------------------------------------------------------
 *
 *  �֐���   : main
 *
 *  ��Ɠ��e : �V�F���̃v�����v�g����������
 *
 *  ����     :
 *
 *  �Ԃ�l   :
 *
 *  ����     :
 *
 *--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char command_buffer[BUFLEN]; /* �R�}���h�p�̃o�b�t�@ */
    char *args[MAXARGNUM];       /* �����ւ̃|�C���^�̔z�� */
    int command_status;          /* �R�}���h�̏�Ԃ�\��

                                    command_status = 0 : �t�H�A�O���E���h�Ŏ��s
                                    command_status = 1 : �o�b�N�O���E���h�Ŏ��s
                                    command_status = 2 : �V�F���̏I��
                                    command_status = 3 : �������Ȃ� */
    AIL_LIST *p;
    DIR_LIST *q;
    HIST_LIST *r;

    /*
     *  �����Ƀ��[�v����
     */

    for(;;) {

        /*
         *  �v�����v�g��\������
         */

        printf("%s : ",prompt);

        /*
         *  �W�����͂���P�s�� command_buffer �֓ǂݍ���
         *  ���͂������Ȃ���Ή��s���o�͂��ăv�����v�g�\���֖߂�
         */


        if(fgets(command_buffer,BUFLEN,stdin) == NULL) {
            printf("\n");
            exit(0);
        }

        /*
         *  ���͂��ꂽ�o�b�t�@���̃R�}���h����͂���
         *
         *  �Ԃ�l�̓R�}���h�̏��
         */

        command_status = parse(command_buffer, args);


        /*
         *  �I���R�}���h�Ȃ�΃v���O�������I��
         *  �����������Ȃ���΃v�����v�g�\���֖߂�
         */

        if(command_status == 2) {
            printf("done.\n");
            exit(EXIT_SUCCESS);
        } else if(command_status == 3) {
            continue;
        }

        /*
         *  �R�}���h�����s����
         */

        execute_command(args, command_status);
    }


    while(1){
      if(lead==NULL)
	break;
      else if(lead->next==NULL){
        free(lead);
        lead=NULL;
        break;
      }
      else{
        p=lead->next;
        p->prev=NULL;
        free(lead);
        lead=p;
      }
    }

    while(1){
      if(root==NULL)
	break;
      else if(root->next==NULL){
        free(root);
        root=NULL;
        break;
      }
      else{
        q=root->next;
        q->prev=NULL;
        free(root);
        root=q;
      }
    }

    while(1){
      if(head==NULL)
	break;
      else if(head->next==NULL){
        free(head);
        head=NULL;
        break;
      }
      else{
        r=head->next;
        r->prev=NULL;
        free(head);
        head=r;
      }
    }

    

    return 0;
}

/*----------------------------------------------------------------------------
 *
 *  �֐���   : parse
 *
 *  ��Ɠ��e : �o�b�t�@���̃R�}���h�ƈ�������͂���
 *
 *  ����     :
 *
 *  �Ԃ�l   : �R�}���h�̏�Ԃ�\�� :
 *                0 : �t�H�A�O���E���h�Ŏ��s
 *                1 : �o�b�N�O���E���h�Ŏ��s
 *                2 : �V�F���̏I��
 *                3 : �������Ȃ�
 *
 *  ����     :
 *
 *--------------------------------------------------------------------------*/

int parse(char buffer[],        /* �o�b�t�@ */
          char *args[])         /* �����ւ̃|�C���^�z�� */
{
    int arg_index;   /* �����p�̃C���f�b�N�X */
    int status;   /* �R�}���h�̏�Ԃ�\�� */

    /*
     *  �ϐ��̏�����
     */

    arg_index = 0;
    status = 0;

    /*
     *  �o�b�t�@���̍Ō�ɂ�����s���k�������֕ύX
     */

    *(buffer + (strlen(buffer) - 1)) = '\0';

    /*
     *  �o�b�t�@���I����\���R�}���h�i"exit"�j�Ȃ��
     *  �R�}���h�̏�Ԃ�\���Ԃ�l�� 2 �ɐݒ肵�ă��^�[������
     */

    if(strcmp(buffer, "exit") == 0) {

        status = 2;
        return status;
    }

    /*
     *  �o�b�t�@���̕������Ȃ��Ȃ�܂ŌJ��Ԃ�
     *  �i�k���������o�Ă���܂ŌJ��Ԃ��j
     */

    while(*buffer != '\0') {

        /*
         *  �󔒗ށi�󔒂ƃ^�u�j���k�������ɒu��������
         *  ����ɂ���ăo�b�t�@���̊e���������������
         */

        while(*buffer == ' ' || *buffer == '\t') {
            *(buffer++) = '\0';
        }

        /*
         * �󔒂̌オ�I�[�����ł���΃��[�v�𔲂���
         */

        if(*buffer == '\0') {
            break;
        }

        /*
         *  �󔒕����͓ǂݔ�΂��ꂽ�͂�
         *  buffer �͌��݂� arg_index + 1 �߂̈����̐擪���w���Ă���
         *
         *  �����̐擪�ւ̃|�C���^�������ւ̃|�C���^�z��Ɋi�[����
         */

        args[arg_index] = buffer;
        ++arg_index;

        /*
         *  ����������ǂݔ�΂�
         *  �i�k�������ł��󔒗ނł��Ȃ��ꍇ�ɓǂݐi�߂�j
         */

        while((*buffer != '\0') && (*buffer != ' ') && (*buffer != '\t')) {
            ++buffer;
        }
    }

    /*
     *  �Ō�̈����̎��ɂ̓k���ւ̃|�C���^���i�[����
     */

    args[arg_index] = NULL;

    /*
     *  �Ō�̈������`�F�b�N���� "&" �Ȃ��
     *
     *  "&" ������������
     *  �R�}���h�̏�Ԃ�\�� status �� 1 ��ݒ肷��
     *
     *  �����łȂ���� status �� 0 ��ݒ肷��
     */

    if(arg_index > 0 && strcmp(args[arg_index - 1], "&") == 0) {

        --arg_index;
        args[arg_index] = '\0';
        status = 1;

    } else {

        status = 0;

    }

    /*
     *  �����������Ȃ������ꍇ
     */

    if(arg_index == 0) {
        status = 3;
    }

    /*
     *  �R�}���h�̏�Ԃ�Ԃ�
     */

    return status;
}

/*----------------------------------------------------------------------------
 *
 *  �֐���   : execute_command
 *
 *  ��Ɠ��e : �����Ƃ��ė^����ꂽ�R�}���h�����s����
 *             �R�}���h�̏�Ԃ��t�H�A�O���E���h�Ȃ�΁A�R�}���h��
 *             ���s���Ă���q�v���Z�X�̏I����҂�
 *             �o�b�N�O���E���h�Ȃ�Ύq�v���Z�X�̏I����҂�����
 *             main �֐��ɕԂ�i�v�����v�g�\���ɖ߂�j
 *
 *  ����     :
 *
 *  �Ԃ�l   :
 *
 *  ����     :
 *
 *--------------------------------------------------------------------------*/

void execute_command(char *args[],    /* �����̔z�� */
                     int command_status)     /* �R�}���h�̏�� */
{
    int pid;      /* �v���Z�X�h�c */
    int status;   /* �q�v���Z�X�̏I���X�e�[�^�X */
    char path[30]; /* �p�X */
    int i;
    clock_t start,end;

    start=clock();

    

    /*
     *  �q�v���Z�X�𐶐�����
     *
     *  �����ł��������m�F���A���s�Ȃ�΃v���O�������I������
     */

    /******** Your Program ********/
      head=extend(args,head);//history�̉������炦
      counter++;
      if(counter>32)rm_hist();//32���傫���Ȃ�����Ԉ���
      for(i=0;args[i]!=NULL;i++){
	if(strcmp(args[i],"*")==0)asta_trans(i,args,command_status);
      }
      if(lead!=NULL)repla(args,lead);
      

    pid = fork();

    if(pid == -1){
      printf("Error:fork failure");
      exit(-1);
    }

    

    /*
     *  �q�v���Z�X�̏ꍇ�ɂ͈����Ƃ��ė^����ꂽ���̂����s����
     *
     *  �����̔z��͈ȉ������肵�Ă���
     *  �E��P�����ɂ͎��s�����v���O���������������񂪊i�[����Ă���
     *  �E�����̔z��̓k���|�C���^�ŏI�����Ă���
     */

    /******** Your Program ********/
    if(pid == 0){
      if(strcmp(args[0],"cd")==0)exit(0);
      else if(strcmp(args[0],"pushd")==0)exit(0);
      else if(strcmp(args[0],"dirs")==0)exit(0);
      else if(strcmp(args[0],"popd")==0)exit(0);
      else if(strcmp(args[0],"history")==0)exit(0);
      else if(strcmp(args[0],"!!")==0)exit(0);
      else if(strchr(args[0],'!')!=NULL)exit(0);
      else if(strcmp(args[0],"prompt")==0)exit(0);
      else if(strcmp(args[0],"alias")==0)exit(0);
      else if(strcmp(args[0],"unalias")==0)exit(0);
      else if(strcmp(args[0],"LU")==0)exit(0);

      sprintf(path,"%s",args[0]);
      execvp(path,args);
      exit(1);
    }
    /*
     *  �R�}���h�̏�Ԃ��o�b�N�O���E���h�Ȃ�֐��𔲂���
     */
   
    /******** Your Program ********/
    if(command_status==1)return;
    /*
     *  �����ɂ���̂̓R�}���h�̏�Ԃ��t�H�A�O���E���h�̏ꍇ
     *
     *  �e�v���Z�X�̏ꍇ�Ɏq�v���Z�X�̏I����҂�
     */

    /******** Your Program ********/
    if(pid != 0){
      wait(&status);



      if(strcmp(args[0],"cd")==0)cd_comannd(args);
      else if(strcmp(args[0],"pushd")==0)pushd_comannd();
      else if(strcmp(args[0],"dirs")==0)dirs_comannd(root);
      else if(strcmp(args[0],"popd")==0)popd_comannd();
      else if(strcmp(args[0],"history")==0)history_comannd(head);
      else if(strcmp(args[0],"!!")==0)ex_comannd(command_status);
      else if(strchr(args[0],'!')!=NULL)exstr_comannd(args,command_status);
      else if(strcmp(args[0],"prompt")==0){
	if(args[1]==NULL)strcpy(prompt,"prompt");
	else  strcpy(prompt,args[1]);
      }
      else if(strcmp(args[0],"alias")==0)alias_comannd(args);
      else if(strcmp(args[0],"unalias")==0)unalias_comannd();
      else if(strcmp(args[0],"keisoku")==0)t=1;
      else if(strcmp(args[0],"keisokuyame")==0)t=0;

   }

    end=clock();

    if(t==1)
      printf("execution time: %d\n",end-start);

    
    return;
}

//����֐�

void cd_comannd(char *args[]){
  if(args[1] == NULL){
  if(chdir(getenv("HOME")) <0)
  perror("cd");
  }
  else{
  if(chdir(args[1]) <0)
    perror("cd");
  }
}

DIR_LIST *new_item(DIR_LIST *post_item, DIR_LIST *pre_item)
{
    DIR_LIST *new;
    char buf[100];

    new=(DIR_LIST *)malloc(sizeof(DIR_LIST));

    getcwd(buf,sizeof(buf));

    strcpy(new->name,buf);
    


    /*** new �̎��̗v�f�̎w�� ***/
    new->next = post_item;
 
    /*** post_item �̑O�̗v�f�̎w��ipost_item �� NULL �łȂ���΁j***/
    if(post_item!=NULL){ post_item->prev =new ;} 
 
    /*** new �̑O�̗v�f�̎w�� ***/
    new->prev = pre_item;

    /*** pre_item �̎��̗v�f�̎w��ipre_item �� NULL �łȂ���΁j ***/
    if(pre_item!=NULL){ pre_item->next = new;}
    return new;
}

DIR_LIST *insert(DIR_LIST*a){


  /* root �� NULL �̏ꍇ�̐V�K���X�g�v�f�̑}������ */
  if(a == NULL)
    return(new_item(NULL,NULL));

  else{
    return(new_item(a, NULL));
  }
}


void pushd_comannd(void){
  /*���X�g�ɒǉ�*/
  root=insert(root);
}

void dirs_comannd(DIR_LIST*a){
  
  /***root �� NULL �Ȃ�߂� ***/
  if(a==NULL)return;

  /*** �t�@�C�����̏o�� ***/
  printf("%s\n",a->name);

  /*** ���̗v�f�̏o�� ***/
  dirs_comannd(a->next);
  return;


}

void popd_comannd(void){
  DIR_LIST* p;
  if(root->next==NULL){
    free(root);
    root=NULL;
  }
  else{
  p=root->next;
  p->prev=NULL;
  free(root);
  root=p;
  }
}

HIST_LIST *new_thing(char *args[],HIST_LIST *post_thing, HIST_LIST *pre_thing)
{
    HIST_LIST *new;
    char *tmp=args[0];
    int i;

    new=(HIST_LIST *)malloc(sizeof(HIST_LIST));

    for(i=1;args[i]!=NULL;i++){
      sprintf(tmp,"%s %s",tmp,args[i]);
    }

    strcpy(new->name,tmp);

    i=1;

    tmp=strtok(tmp," ");
     while(tmp!=NULL){
       tmp=strtok(NULL," ");
       args[i]=tmp;
       i++;
     }
    


    /*** new �̎��̗v�f�̎w�� ***/
    new->next = post_thing;
 
    /*** post_item �̑O�̗v�f�̎w��ipost_item �� NULL �łȂ���΁j***/
    if(post_thing!=NULL){ post_thing->prev =new ;} 
 
    /*** new �̑O�̗v�f�̎w�� ***/
    new->prev = pre_thing;
    
    /*** pre_item �̎��̗v�f�̎w��ipre_item �� NULL �łȂ���΁j ***/
    if(pre_thing!=NULL){ pre_thing->next = new;}
    return new;
}

HIST_LIST *extend(char *args[],HIST_LIST*a){


  /* root �� NULL �̏ꍇ�̐V�K���X�g�v�f�̑}������ */
  if(a == NULL)
    return(new_thing(args,NULL,NULL));

  else{
    return(new_thing(args,a, NULL));
  }
}

void rm_hist(void){
  HIST_LIST* p;

  while(head->next!=NULL)head=head->next;
  
  if(head->prev==NULL){
    free(head);
    head=NULL;
  }
  else{
  p=head->prev;
  p->next=NULL;
  free(head);
  head=p;
  }

  while(head->prev!=NULL)head=head->prev;
}

void history_comannd(HIST_LIST *head){
  int i;
  if(counter<32)i=1;
  else{i=counter-32;}
  
  while(head->next!=NULL)
    head=head->next;
  for(head;head!=NULL;head=head->prev){
      printf("%d %s\n",i,head->name);
      i++;
  }
}

void ex_comannd(int b){
   HIST_LIST* p;
   char *a[MAXARGNUM],*c;
   int i=0;

   //�u!!�v�����X�g����폜  
  if(head->next==NULL){
    free(head);
    head=NULL;
  }
  else{
  p=head->next;
  p->prev=NULL;
  free(head);
  head=p;
  }
  
 
  c=strtok(head->name," ");
  a[0]=c;
  while(c!=NULL){
    c=strtok(NULL," ");
    i++;
    a[i]=c;
  }  
 

  execute_command(a,b);
  
}

void exstr_comannd(char *args[],int b){
   HIST_LIST* p;
   char *a[MAXARGNUM],*c,*d;
   int i=0;

   //�u!string�v�����X�g����폜  
  if(head->next==NULL){
    free(head);
    head=NULL;
  }
  else{
  p=head->next;
  p->prev=NULL;
  free(head);
  head=p;
  }

  d=strtok(args[0],"!");

  while(p->next!=NULL){
    if(strstr(p->name,d)!=NULL)break;
    p=p->next;
  }
  printf("%s\n",p->name);
  
  c=strtok(p->name," ");
  a[0]=c;
  while(c!=NULL){
    c=strtok(NULL," ");
    i++;
    a[i]=c;
  }  

  execute_command(a,b);
  

}

void *asta_trans(int i,char *args[],int command_status){
  FILE *fp;
  DIR  *dp;
  struct stat filestat;
  struct dirent *dir;
  char buf[MAXARGNUM],buffer[BUFLEN],tmtmtmp[MAXARGNUM];
  char *tmp=args[0],*tmtmp;
  char *a,*b;
  int j=0,ii=i,k;
  HIST_LIST *p;

  if(strcmp(args[0],"*")==0){
    printf("incorrect input\n");
    return;
  }

  
  if(head->next==NULL){
    free(head);
    head=NULL;
  }
  else{
  p=head->next;
  p->prev=NULL;
  free(head);
  head=p;
  }


    for(j=1;args[j]!=NULL;j++){
      sprintf(tmp,"%s %s",tmp,args[j]);//��x���̈�s�̌`�ɖ߂�
    }
  

    tmp=strtok(tmp,"*");//��s��*�ŕ������đO������tmp��
    tmtmp=strtok(NULL,"\0");//��땔����tmtmp��
    
    if(tmtmp!=NULL)
    strcpy(tmtmtmp,tmtmp);

    

  getcwd(buf,sizeof(buf));

  dp=opendir(buf);



  while((dir=readdir(dp))!=NULL){
    if(!strcmp(dir->d_name, ".") ||
       !strcmp(dir->d_name, ".."))
      continue;
    if(stat(dir->d_name,&filestat)!=0){
      perror("main");
      exit(1);
    }else{
      if(S_ISREG(filestat.st_mode)){
	sprintf(tmp,"%s %s",tmp,dir->d_name);
      }
    }
  }



  closedir(dp);


  tmtmp=tmtmtmp;



  sprintf(tmp,"%s %s",tmp,tmtmp);
  
  strcpy(buffer,tmp);


    k=parse(buffer,args);

    execute_command(args,k);
}

AIL_LIST *new_staff(char *args[],AIL_LIST *post_staff,AIL_LIST *pre_staff){
  AIL_LIST *new;
  char *a;

  new=(AIL_LIST *)malloc(sizeof(AIL_LIST));



  a=args[1];
  strcpy(new->post_name,a);
  a=args[2];
  strcpy(new->pre_name,a);




    /*** new �̎��̗v�f�̎w�� ***/
    new->next = post_staff;
 
    /*** post_item �̑O�̗v�f�̎w��ipost_item �� NULL �łȂ���΁j***/
    if(post_staff!=NULL){ post_staff->prev =new ;} 
 
    /*** new �̑O�̗v�f�̎w�� ***/
    new->prev = pre_staff;
    
    /*** pre_item �̎��̗v�f�̎w��ipre_item �� NULL �łȂ���΁j ***/
    if(pre_staff!=NULL){ pre_staff->next = new;}


    
    return new;
  
  
}

void alias_comannd(char *args[]){

    if(args[1]==NULL){
      if(lead==NULL){
	printf("you have not defined any command again\n");
	return;
      }
      else{

        while(1){
          printf("%s %s\n",lead->post_name,lead->pre_name);
	  if(lead->next!=NULL)
	  lead=lead->next;
	  else break;
        }

        while(lead->prev!=NULL)//�擪�܂ŋA��
        lead=lead->prev;

      }
      return;
    }
  else{

    if(lead==NULL){

     lead=(new_staff(args,NULL,NULL));
    }
    else{

      lead=(new_staff(args,lead,NULL));
    }
  }  
}

void repla(char *args[],AIL_LIST *a){

    while(1){

    if(strcmp(args[0],a->post_name)==0){

      strcpy(args[0],a->pre_name);
      break;
    }
    if(a->next!=NULL)
    a=a->next;
    else break;
  };
  
  while(a->prev!=NULL)a=a->prev;
  


}

void unalias_comannd(void){
  AIL_LIST *p;
  if(lead==NULL){
    printf("you have not defined any commands\n again");
    return;
  }
  while(1){
    if(lead->next==NULL){
      free(lead);
      lead=NULL;
      break;
    }
    else{
      p=lead->next;
      p->prev=NULL;
      free(lead);
      lead=p;
    }
  }
}


/*-- END OF FILE -----------------------------------------------------------*/
