#define SNAKE_SYMBOL '@'
#define FOOD_SYMBOL '*'
#define MAX_NODE 30
#define DFL_SPEED 50
#define TOP_ROW 5
#define BOT_ROW LINES-1
#define LEFT_EDGE 0
#define RIGHT_EDGE COLS-1

typedef struct node  //蛇的结构
{
    /* data */
    int x_pos;
    int y_pos;
    struct node *prev;
    struct node *next;
}Snake_Node;

struct position  //食物的位置
{
    /* data */
    int x_pos;
    int y_pos;
};

void Init_Disp(); //初始化并显示接口
void Food_Disp(); //显示食物位置
void Wrap_Up();   // 关闭诅咒
void Key_Ctrl(); //使用键盘控制蛇
int set_ticker(int n_msecs);//钟表

void DLL_Snake_Create();    //创建双向链表
void DLL_Snake_Insert(int x,int y); //插入节点
void DLL_Snake_Delete_Node(); //删除节点
void DLL_Snake_Delete();//删除整个双向链表

void Snake_Move(int sig);   //控制蛇的移动和判断
void gameover(int n); //不同n表示不同状态









