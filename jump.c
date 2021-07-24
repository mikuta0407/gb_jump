#include <stdio.h>
#include <gb/gb.h>
#include <gb/drawing.h>
#include <time.h>
#include <rand.h>
#include "projects/player.c"
#include "projects/enm.c"

//プレイヤー位置初期化
UBYTE x = 32;
UBYTE y = 100;

//ジャンプ関連
int canJump = 1;
int f = -1;
int y_ground = -1000;
int y_prev = 100;
int y_tmp;



//モード遷移
//0: スタートメニュー 1: ゲーム 2: GameOver
UBYTE gamemode = 0;

//ゲーム中変数
UBYTE isGameOver = 0;
int timecount = 0;
UBYTE obj_ytmp;
//プロトタイプ宣言
void start();
void game();



void main() {

    
    
    printf("Push start!\n");
    while(1){
        start();
        if (joypad() & J_START){
            printf("START\n");
            break;
        }
    }

    game();

    //gameover();
    
}

void start(){

}


void game(){

    //図形描画位置
    //地面    
    UBYTE line_x1, line_x2, line_y1, line_y2;
    //敵
    int obj_speed = 4;
    int obj_x = 161;
    UBYTE obj_y = 70;
    UBYTE loto;
    

    //スタート状態管理
    //0: カウントダウン
    //1: スタート
    UBYTE start = 0;

    //時間管理
    time_t now_time = 0;
    time_t prev_time = 0;
    UBYTE count = 0;
    UBYTE score = 0;

    //printf("hello\n"); //debug

    SPRITES_8x16;
	set_sprite_data(0, 0, player);
	set_sprite_tile(0, 0);
    move_sprite(0, x, y);

    set_sprite_data(2, 2, enm);
	set_sprite_tile(2, 2);
    move_sprite(2, obj_x, obj_y);

	SHOW_SPRITES;

    //Draw Ground
    line_x1 = 0;
    line_y1 = 100;
    line_x2 = 160;
    line_y2 = 100;
    color(DKGREY, WHITE, SOLID);/* 色をダークグレーに設定 */
    line(0, 100, 159, 100);       /* 線を引く */

    initrand(arand());
    

	while(1) {

        loto = rand();

        //時間計測
        if (sys_time - prev_time >= 60){
            count++;
            prev_time = sys_time;
            if(start == 1){
                //時間表示
                gotogxy(0, 0);
                color(3, 0, SOLID);
                gprintf("TIME: %d",count);
            }
        }
        

        if (start == 0){
            if (count <= 3){ //3秒カウントダウン
                gotogxy(9, 8);
                color(3, 0, SOLID);
                gprintf("%d", 4 - count);
            } else if (count <= 4){ //START!!を出す
                gotogxy(7, 8);
                color(3, 0, SOLID);
                gprintf("START!!");
                
            } else if (count <= 5){ //表示を消してゲームスタート
                gotogxy(7, 8);
                color(3, 0, SOLID);
                gprintf("         ");
                start = 1;
                count = 0;
            }
        }

        if (start == 1){

            

            /*//スコア表示
            gotogxy(0, 1);
            color(3, 0, SOLID);
            gprintf("SCORE: %d",score);
            */

            //敵物体処理
            gotogxy(0, 2);
            color(3, 0, SOLID);
            gprintf("objy: %d    ",obj_y);
            

            if (obj_x >= 160){
                //抽選用乱数決定
                loto = rand();
                gotogxy(0, 3);
                color(3, 0, SOLID);
                gprintf("loto:%d    ",(int)loto);
                
                
                if ((int)loto <= 0){
                    obj_ytmp = 100;
                } 
                if ((int)loto > 0){
                    obj_ytmp = 70;
                }
                obj_x = 159;
                obj_y = obj_ytmp;
            }

            if ( obj_x < 160 && obj_x >= 0){
                obj_x -= obj_speed;
            
            } 
            if (obj_x < 0){
                obj_x = 161;
            }


            //操作部分
            if(joypad() & J_A) {
                if (canJump == 1){
                    f = -10;
                    y_ground = y;
                    canJump = 0;
                }
            }

            if (canJump == 0){
                y_tmp = y;
                y += (y - y_prev) + f;
                y_prev = y_tmp;
                f = 1;
            }

            if (y == y_ground){
                canJump = 1;
            }
            move_sprite(0, x, y);
            move_sprite(2, obj_x, obj_y);
        }

        if (isGameOver == 1){
            break;
        }

        
		//wait_vbl_done();
	}
}