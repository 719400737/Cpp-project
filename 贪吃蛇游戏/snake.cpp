#include"snake.h"
#include<ncurses.h>
#include<stdlib.h>
#include<sys/time.h>
#include<stdio.h>
#include<signal.h>
#include<time.h>

struct position food;
Snake_Node *head,*tail;
int x_dir=1,y_dir=0;
int ttm=5,ttg=5;

int main(){
    Init_Disp(); //初始化界面
    Food_Disp(); //初始化食物界面
    DLL_Snake_Create();//初始化蛇
    signal(SIGALRM,Snake_Move);//给当前进程注册sigalrm信号处理代码，如果收到信号则响应信号
    set_ticker(DFL_SPEED); //定期发出信号，蛇移动
    Key_Ctrl(); //while 循环，检测键盘输入，控制蛇的移动
    Wrap_Up();
    return 0;
}
void Init_Disp(){
    char wall=' ';
    int i,j;
    initscr(); //初始化库
    cbreak();  //开启输入立即相应
    noecho();  //输入不会显示到屏幕
    curs_set(0);//光标不可见

    attrset(A_NORMAL); /* 先将属性设定为正常模式 */
    attrset(A_REVERSE); /* 开启反白模式 */
    for(i=0;i<LINES;i++){
        mvaddch(i,LEFT_EDGE,wall);//在(x,y) 上显示某个字元. 相当於呼叫move(y,x);addch(ch);
        mvaddch(i,RIGHT_EDGE,wall);
    }
    for(j=0;j<COLS;j++){
        mvaddch(TOP_ROW,j,wall);
        mvaddch(BOT_ROW,j,wall);
    }
    attroff(A_REVERSE);
    mvaddstr(1,2,"Game snake version:1.0");
    mvaddstr(2,2,"Author:hange");
    mvaddstr(3,2,"'f' to speed up,'s' to speed down,'q' to quit");
    refresh();


}

void Food_Disp(){
    srand(time(0));
    food.x_pos=rand()%(COLS-2)+1;
    food.y_pos=rand()%(LINES-TOP_ROW-2)+TOP_ROW;
    mvaddch(food.y_pos,food.x_pos,FOOD_SYMBOL);
    refresh();
}

void DLL_Snake_Create(){
    Snake_Node *temp=(Snake_Node *)malloc(sizeof(Snake_Node));
    head=(Snake_Node *)malloc(sizeof(Snake_Node));
    tail=(Snake_Node *)malloc(sizeof(Snake_Node));
    if(temp==NULL||head==NULL||tail==NULL)
        perror("malloc");
    temp->x_pos=5;
    temp->y_pos=10;
    head->prev=NULL;
    tail->next=NULL;
    head->next=temp;
    temp->next=tail;
    tail->prev=temp;
    temp->prev=head;
    mvaddch(temp->y_pos,temp->x_pos,SNAKE_SYMBOL);
    refresh();
}
void Snake_Move(int signum){
    static int length=1;
    int Length_Flag=0;
    int moved=0;
    signal(SIGALRM,SIG_IGN);//SIG_DFL:信号由该特定信号的默认动作处理
                            //DIG_IGN：忽略信号
                            //sigalrm 表示alarm设置的timer超时
    if((head->next->x_pos==RIGHT_EDGE-1 && x_dir==1) //判断是否撞墙 x_dir表示方向
        ||(head->next->x_pos==LEFT_EDGE+1 && x_dir==-1)  //head->next 表示蛇头位置
        ||(head->next->y_pos==TOP_ROW+1 && y_dir==-1)
        ||(head->next->y_pos==BOT_ROW-1 && y_dir==1))
    {
        gameover(1);
    }
    if(mvinch(head->next->y_pos+y_dir,head->next->x_pos+x_dir)=='@') //碰到自己的身体
        gameover(2);
    
    if(ttm>0 && ttg--==1){ //先执行表达式在--
        DLL_Snake_Insert(head->next->x_pos+x_dir,head->next->y_pos+y_dir);
        ttg=ttm;  //用来控制速度,当ttg为1时才插入节点
        moved=1;
    }
    if(moved){
        //蛇吃食物
        if(head->next->x_pos==food.x_pos && head->next->y_pos==food.y_pos){
            Length_Flag=1; //吃到了食物，Flag变为1
            length++;
            if(length>=MAX_NODE)
                gameover(0);
            Food_Disp();
        }
        if(Length_Flag==0){
            mvaddch(tail->prev->y_pos,tail->prev->x_pos,' ');//尾节点前一个表示蛇的最后一个节点
            DLL_Snake_Delete_Node();//需要删除最后一个节点。
        }
        mvaddch(head->next->y_pos,head->next->x_pos,SNAKE_SYMBOL);
        refresh();
    }
    signal(SIGALRM,Snake_Move);//sigalrm是一个时钟信号，时间到了内核会向进程发送这个信号。
    //退出函数之前，继续捕捉信号。
    
}
//该函数安排间隔定时起定期发布 SIGALRM 信号
int set_ticker(int n_msecs){  //参数为蛇的速度，定期发送信号
    struct itimerval new_timeset;
    long n_sec,n_usecs; //将毫秒转化为秒和微秒
    n_sec=n_msecs/1000; //秒部分
    n_usecs=(n_msecs%1000)*1000L;//微秒部分
    new_timeset.it_interval.tv_sec=n_sec; //每两次定时器生效的时间间隔。
    new_timeset.it_interval.tv_usec=n_usecs;

    new_timeset.it_value.tv_sec=n_sec;  //从设定定时器开始计算，到第一次定时器生效的时间
    new_timeset.it_value.tv_usec=n_usecs;

    return setitimer(ITIMER_REAL,&new_timeset,NULL);
    //ITIMER_REAL 表示系统真实的时间，它送出SIGALRM信号，
    //该函数工作机制先对it_value进行倒计时，当其为0时触发信号，然后将其重置it_interval，继续对value计时，一直重复下去。
    
}

void Wrap_Up(){
    set_ticker(0); // 关闭计时器
    getchar();
    endwin(); //关闭curse
    exit(0);
}
void Key_Ctrl(){
    int c;
    keypad(stdscr,true); //开启小键盘捕获
    while(c=getch(),c!='q'){
        if(c=='f'){
            if(ttm==1)
                continue;
            ttm--; //ttm越小越快
        }
        else if(c=='s'){
            if(ttm==8)
                continue;
            ttm++;
        }
        if(c==KEY_LEFT){
            if(tail->prev->prev->prev != NULL && x_dir==1 && y_dir==0)
                continue; //当蛇长度大于1时，不能翻转
            x_dir=-1;
            y_dir=0;
        }
        else if(c==KEY_RIGHT){
            if(tail->prev->prev->prev != NULL && x_dir==-1 && y_dir==0)
                continue;
            x_dir=1;
            y_dir=0;
        }
        else if(c == KEY_UP)
		{
			if(tail->prev->prev->prev != NULL && x_dir == 0 && y_dir == 1)
				continue;
			x_dir = 0;
			y_dir = -1;
		}
		else if(c == KEY_DOWN)
		{
			if(tail->prev->prev->prev != NULL && x_dir == 0 && y_dir == -1)
				continue;
			x_dir = 0;
			y_dir = 1;
		}

    }
}

void DLL_Snake_Insert(int x,int y){
    Snake_Node *temp=(Snake_Node *)malloc(sizeof(Snake_Node));
    if(temp==NULL)
        perror("malloc");
    temp->x_pos=x;
    temp->y_pos=y;
    temp->prev=head->next->prev;
    head->next->prev=temp;
    temp->next=head->next;
    head->next=temp;  
}


void gameover(int n){
    switch (n)
    {
        case 0:
            mvaddstr(LINES/2,COLS/3-4,"mission completes,any key to exit.\n");
            break;
        case 1:
			mvaddstr(LINES/2, COLS/3 - 4, "Game Over, crash the wall,press any key to exit.\n");
			break;
		case 2:
			mvaddstr(LINES/2, COLS/3 - 4, "Game Over, crash yourself,press any key to exit.\n");
            break;
    default:
        break;
    }
    refresh();
    DLL_Snake_Delete();
    Wrap_Up();
}

void DLL_Snake_Delete_Node(){
    Snake_Node *temp;
    temp=tail->prev;
    tail->prev=temp->prev;
    temp->prev->next=tail;
    free(temp);


}

void DLL_Snake_Delete(){
    while(head->next!=tail){
        DLL_Snake_Delete_Node();
    }
    head->next=tail->prev=NULL;
    free(head);
    free(tail);
    
}