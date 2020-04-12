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
 *  定数の定義
 */

#define BUFLEN    1024     /* コマンド用のバッファの大きさ */
#define MAXARGNUM  256     /* 最大の引数の数 */

/*
 *  ローカルプロトタイプ宣言
 */

char prompt[50]="Command";     /* プロンプト*/


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

DIR_LIST *root=NULL; /* DIR_LISTの先頭 */
HIST_LIST *head=NULL;/* HIST_LISTの先頭 */
AIL_LIST  *lead=NULL;/* AIL_LISTの先頭*/
int counter=0;       /*history_comanndのカウンタ*/
int t=0;             /*自作関数用のフラグ*/

int parse(char [], char *[]);
void execute_command(char *[], int);
//試作関数
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
 *  関数名   : main
 *
 *  作業内容 : シェルのプロンプトを実現する
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char command_buffer[BUFLEN]; /* コマンド用のバッファ */
    char *args[MAXARGNUM];       /* 引数へのポインタの配列 */
    int command_status;          /* コマンドの状態を表す

                                    command_status = 0 : フォアグラウンドで実行
                                    command_status = 1 : バックグラウンドで実行
                                    command_status = 2 : シェルの終了
                                    command_status = 3 : 何もしない */
    AIL_LIST *p;
    DIR_LIST *q;
    HIST_LIST *r;

    /*
     *  無限にループする
     */

    for(;;) {

        /*
         *  プロンプトを表示する
         */

        printf("%s : ",prompt);

        /*
         *  標準入力から１行を command_buffer へ読み込む
         *  入力が何もなければ改行を出力してプロンプト表示へ戻る
         */


        if(fgets(command_buffer,BUFLEN,stdin) == NULL) {
            printf("\n");
            exit(0);
        }

        /*
         *  入力されたバッファ内のコマンドを解析する
         *
         *  返り値はコマンドの状態
         */

        command_status = parse(command_buffer, args);


        /*
         *  終了コマンドならばプログラムを終了
         *  引数が何もなければプロンプト表示へ戻る
         */

        if(command_status == 2) {
            printf("done.\n");
            exit(EXIT_SUCCESS);
        } else if(command_status == 3) {
            continue;
        }

        /*
         *  コマンドを実行する
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
 *  関数名   : parse
 *
 *  作業内容 : バッファ内のコマンドと引数を解析する
 *
 *  引数     :
 *
 *  返り値   : コマンドの状態を表す :
 *                0 : フォアグラウンドで実行
 *                1 : バックグラウンドで実行
 *                2 : シェルの終了
 *                3 : 何もしない
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int parse(char buffer[],        /* バッファ */
          char *args[])         /* 引数へのポインタ配列 */
{
    int arg_index;   /* 引数用のインデックス */
    int status;   /* コマンドの状態を表す */

    /*
     *  変数の初期化
     */

    arg_index = 0;
    status = 0;

    /*
     *  バッファ内の最後にある改行をヌル文字へ変更
     */

    *(buffer + (strlen(buffer) - 1)) = '\0';

    /*
     *  バッファが終了を表すコマンド（"exit"）ならば
     *  コマンドの状態を表す返り値を 2 に設定してリターンする
     */

    if(strcmp(buffer, "exit") == 0) {

        status = 2;
        return status;
    }

    /*
     *  バッファ内の文字がなくなるまで繰り返す
     *  （ヌル文字が出てくるまで繰り返す）
     */

    while(*buffer != '\0') {

        /*
         *  空白類（空白とタブ）をヌル文字に置き換える
         *  これによってバッファ内の各引数が分割される
         */

        while(*buffer == ' ' || *buffer == '\t') {
            *(buffer++) = '\0';
        }

        /*
         * 空白の後が終端文字であればループを抜ける
         */

        if(*buffer == '\0') {
            break;
        }

        /*
         *  空白部分は読み飛ばされたはず
         *  buffer は現在は arg_index + 1 個めの引数の先頭を指している
         *
         *  引数の先頭へのポインタを引数へのポインタ配列に格納する
         */

        args[arg_index] = buffer;
        ++arg_index;

        /*
         *  引数部分を読み飛ばす
         *  （ヌル文字でも空白類でもない場合に読み進める）
         */

        while((*buffer != '\0') && (*buffer != ' ') && (*buffer != '\t')) {
            ++buffer;
        }
    }

    /*
     *  最後の引数の次にはヌルへのポインタを格納する
     */

    args[arg_index] = NULL;

    /*
     *  最後の引数をチェックして "&" ならば
     *
     *  "&" を引数から削る
     *  コマンドの状態を表す status に 1 を設定する
     *
     *  そうでなければ status に 0 を設定する
     */

    if(arg_index > 0 && strcmp(args[arg_index - 1], "&") == 0) {

        --arg_index;
        args[arg_index] = '\0';
        status = 1;

    } else {

        status = 0;

    }

    /*
     *  引数が何もなかった場合
     */

    if(arg_index == 0) {
        status = 3;
    }

    /*
     *  コマンドの状態を返す
     */

    return status;
}

/*----------------------------------------------------------------------------
 *
 *  関数名   : execute_command
 *
 *  作業内容 : 引数として与えられたコマンドを実行する
 *             コマンドの状態がフォアグラウンドならば、コマンドを
 *             実行している子プロセスの終了を待つ
 *             バックグラウンドならば子プロセスの終了を待たずに
 *             main 関数に返る（プロンプト表示に戻る）
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

void execute_command(char *args[],    /* 引数の配列 */
                     int command_status)     /* コマンドの状態 */
{
    int pid;      /* プロセスＩＤ */
    int status;   /* 子プロセスの終了ステータス */
    char path[30]; /* パス */
    int i;
    clock_t start,end;

    start=clock();

    

    /*
     *  子プロセスを生成する
     *
     *  生成できたかを確認し、失敗ならばプログラムを終了する
     */

    /******** Your Program ********/
      head=extend(args,head);//historyの下ごしらえ
      counter++;
      if(counter>32)rm_hist();//32より大きくなったら間引く
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
     *  子プロセスの場合には引数として与えられたものを実行する
     *
     *  引数の配列は以下を仮定している
     *  ・第１引数には実行されるプログラムを示す文字列が格納されている
     *  ・引数の配列はヌルポインタで終了している
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
     *  コマンドの状態がバックグラウンドなら関数を抜ける
     */
   
    /******** Your Program ********/
    if(command_status==1)return;
    /*
     *  ここにくるのはコマンドの状態がフォアグラウンドの場合
     *
     *  親プロセスの場合に子プロセスの終了を待つ
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

//試作関数

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
    


    /*** new の次の要素の指定 ***/
    new->next = post_item;
 
    /*** post_item の前の要素の指定（post_item が NULL でなければ）***/
    if(post_item!=NULL){ post_item->prev =new ;} 
 
    /*** new の前の要素の指定 ***/
    new->prev = pre_item;

    /*** pre_item の次の要素の指定（pre_item が NULL でなければ） ***/
    if(pre_item!=NULL){ pre_item->next = new;}
    return new;
}

DIR_LIST *insert(DIR_LIST*a){


  /* root が NULL の場合の新規リスト要素の挿入処理 */
  if(a == NULL)
    return(new_item(NULL,NULL));

  else{
    return(new_item(a, NULL));
  }
}


void pushd_comannd(void){
  /*リストに追加*/
  root=insert(root);
}

void dirs_comannd(DIR_LIST*a){
  
  /***root が NULL なら戻る ***/
  if(a==NULL)return;

  /*** ファイル名の出力 ***/
  printf("%s\n",a->name);

  /*** 次の要素の出力 ***/
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
    


    /*** new の次の要素の指定 ***/
    new->next = post_thing;
 
    /*** post_item の前の要素の指定（post_item が NULL でなければ）***/
    if(post_thing!=NULL){ post_thing->prev =new ;} 
 
    /*** new の前の要素の指定 ***/
    new->prev = pre_thing;
    
    /*** pre_item の次の要素の指定（pre_item が NULL でなければ） ***/
    if(pre_thing!=NULL){ pre_thing->next = new;}
    return new;
}

HIST_LIST *extend(char *args[],HIST_LIST*a){


  /* root が NULL の場合の新規リスト要素の挿入処理 */
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

   //「!!」をリストから削除  
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

   //「!string」をリストから削除  
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
      sprintf(tmp,"%s %s",tmp,args[j]);//一度元の一行の形に戻す
    }
  

    tmp=strtok(tmp,"*");//一行を*で分割して前部分をtmpに
    tmtmp=strtok(NULL,"\0");//後ろ部分をtmtmpに
    
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




    /*** new の次の要素の指定 ***/
    new->next = post_staff;
 
    /*** post_item の前の要素の指定（post_item が NULL でなければ）***/
    if(post_staff!=NULL){ post_staff->prev =new ;} 
 
    /*** new の前の要素の指定 ***/
    new->prev = pre_staff;
    
    /*** pre_item の次の要素の指定（pre_item が NULL でなければ） ***/
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

        while(lead->prev!=NULL)//先頭まで帰る
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
