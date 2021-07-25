#include <stdio.h>
#include <gb/gb.h>
#include <gb/drawing.h>
#include <time.h>
#include <rand.h>
#include "projects/player.c"
#include "projects/enm.c"
#include "projects/start_screen.h"

// コンパイルコマンド
// bin\lcc.exe projects\jump.c -o projects\jump.gb


// プレイヤー位置初期化
UBYTE x = 32;
UBYTE y = 100;

// ジャンプ関連
// ジャンプに関しては https://qiita.com/odanny/items/297f32a334c41410cc5d#おまけマリオ を参照
int canJump = 1; // ジャンプ可否変数
int f = -1; // y軸用(詳細は例のQiita)
//int y_ground = -1000; // 地面y座標用。アクションゲームならこっち
int y_ground = 100; // 地面y座標用。今回は固定値なのでこっち
int y_prev = 100; // yの前フレーム位置保存用。
int y_tmp; // ジャンプ処理に必要な仮保存用


// モード遷移
// 0: スタートメニュー 1: ゲーム 2: GameOver
UBYTE gamemode = 0;
int clear;

// ゲーム中変数
UBYTE isGameOver = 0;
UBYTE obj_ytmp;

// スコア(STARTとGAMEOVER時に利用するので)
UBYTE score = 0;

// 時間管理
time_t prev_time;
UBYTE timecount = 0; // 秒数カウント (GAMEOVERで使うので)

// プロトタイプ宣言
void start();
void game();
void gameover();


void main() {

    // スタート画面の描画処理関数
    start();

    // START押されたらbreakしてgame();へ
    while(1){
        if (joypad() & J_START){
            HIDE_BKG;
            break;
        }
    }

    // game()を実行(クリア/ゲームオーバーでbreakされるまで動作)
    game();

    // gameがbreakされる=ゲームオーバーなのでgameover()を実行。
    // gameover()はwhile(1)で回り続ける
    // STARTボタンを押したらreset()するので、プログラムとしてはgameover()の無限ループが最後
    gameover();
    
}

// スタート画面描画
void start(){

    set_bkg_data(0, 255, start_screen_tile_data);
    set_bkg_tiles(0, 0, start_screen_tile_map_width, start_screen_tile_map_height, start_screen_map_data);

    SHOW_BKG;
}

//ゲーム部分
void game(){

    /* 位置関連変数 */
    // 地面    
    UBYTE line_x1, line_x2, line_y1, line_y2;
    
    // 敵位置
    UBYTE obj_speed = 1;
    int obj_x = 159;
    UBYTE obj_y = 100;

    /* ゲーム処理部分変数 */
    UBYTE loto; // 敵上下位置抽選rand値格納
    int threshold = 64; //抽選しきい値。 -127~127 +だと上 -だと下

    // スタート状態管理
    // 0: カウントダウン
    // 1: スタート
    UBYTE start = 0;

    prev_time = sys_time; // 60カウントされるまでの差分確認用

    /* スプライト設定 */
    SPRITES_8x16;
    // プレイヤースプライト設定
	set_sprite_data(0, 0, player);
	set_sprite_tile(0, 0);
    move_sprite(0, x, y);

    // 敵物体スプライト設定
    set_sprite_data(2, 2, enm);
	set_sprite_tile(2, 2);
    move_sprite(2, obj_x, obj_y);

	SHOW_SPRITES;

    /* 地面描画(線) */
    line_x1 = 0;
    line_y1 = 100;
    line_x2 = 160;
    line_y2 = 100;
    color(DKGREY, WHITE, SOLID); // 色をダークグレーに
    line(0, 100, 159, 100); // 線描画

    /* 乱数seed設定 */
    initrand(arand());

    //READY?を表示
    gotogxy(7, 8);
    color(3, 0, SOLID);
    gprintf("READY?");

    //TIMEとSCORE部分を最初に書いておく
    // 時間表示
    gotogxy(0, 0);
    color(3, 0, SOLID);
    gprintf("TIME: 0");
    // スコア表示
    gotogxy(0, 1);
    color(3, 0, SOLID);
    gprintf("SCORE: 0");

	while(1) {

        /* 時間カウント処理 */
        // 60F毎にtimecountをインクリメント→秒時計に
        if (sys_time - prev_time >= 60){
            timecount++;
            prev_time = sys_time;
            if(start == 1){ // gprintfを1秒に一回しか更新しないための処理
                // 時間表示
                gotogxy(0, 0);
                color(3, 0, SOLID);
                gprintf("TIME: %u   ",timecount);
            }
        }
        
        /* カウントダウン中の処理 */
        if (start == 0){

            if ( timecount >= 1 && timecount <= 3){ // 3秒カウントダウン
                gotogxy(7, 8);
                color(3, 0, SOLID);
                gprintf("  %d   ", 4 - timecount); // timecountはインクリメントなので、4から引いて表示
            
            } else if (timecount == 4){ // START!!を出す
                
                gotogxy(6, 8);
                color(3, 0, SOLID);
                gprintf("START!!");
                
            } else if (timecount == 5){ // 表示を消してゲームスタート
                
                gotogxy(6, 8);
                color(3, 0, SOLID);
                gprintf("         ");
                start = 1;
                timecount = 0;

            }
        }

        /* カウントダウン終了後、実ゲーム中 */
        if (start == 1){

            /* 敵物体処理 */

            // x=160以上(画面右端より右。画面外。)
            // 上下抽選処理
            if (obj_x >= 160){

                //抽選用乱数決定
                loto = rand();                
                
                // intにキャストしたほうが確実に処理できたのでキャストして処理
                // しきい値によって上か下かを決定
                if ((int)loto <= threshold){
                    obj_ytmp = 100;
                } 
                if ((int)loto > threshold){
                    obj_ytmp = 70;
                }
                obj_x = 159; // xが0~159の間は値を減らす処理するのでここで159へ
                obj_y = obj_ytmp; // 抽選結果のy値を反映
            }

            // 159~0までの処理
            if ( obj_x < 160 && obj_x >= 0){
                obj_x -= obj_speed; //obj_speedの値に従って左へ移動
                obj_x -= 2;
            }

            // 敵物体が画面左端へ(スコア獲得時)
            if (obj_x < 0){
                // スコアインクリメント
                score++;

                // scoreが15の倍数だった場合はスピードアップ
                if (score % 15 == 0){
                    obj_speed += 1;
                }

                // スコア表示
                gotogxy(0, 1);
                color(3, 0, SOLID);
                gprintf("SCORE: %u   ",score);

                // 最大値でクリア! (時間よりもscoreの方が先に255に行く)
                if (score == 255){
                    clear = 1;
                    break; //gameover()へ
                }

                // 位置リセット(160以上の際の処理へ)
                obj_x = 161;
            }

            //当たり判定
            if (obj_x <= 38 && obj_x >= 30 ){ //まず横方向
                if (obj_y <= y + 8 && obj_y >= y - 8){
                    clear = 0;
                    break; //gameover()へ
                }
            }


            // 操作部分
            if(joypad() & J_A) {
                if (canJump == 1){
                    f = -10;
                    //y_ground = y;
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
        
		wait_vbl_done();
	}
}

void gameover(){
    
    int count = 0; //時間カウント
    int str_x = 5; //文字左端位置

    char congrats_str[10] = "CONGRATS!!";
    char gameover_str[10] = "GAME OVER!";
  
    color(BLACK, WHITE, SOLID);  /* 色を黒に設定 */
    box(39, 30, 121, 110, M_FILL); /* 塗潰し四角形を描く */

    while(count <= 16){ //16カウント分やる

        //色指定
        color(3, 0, SOLID);

        //15F毎に処理
        if (sys_time - prev_time >= 15){
            
            prev_time = sys_time;

            //1カウント毎に配列の中の文字を一個ずつ吐いていく

            if (count >= 1 && count <= 11){
                gotogxy(5 + count - 1, 5);
                if (clear == 1){ //クリア      
                    gprintf("%c",congrats_str[count - 1]);
                }

                if (clear == 0){ //ゲームオーバー
                    gprintf("%c",gameover_str[count - 1]);
                }
            }

            if (count == 13){ //時間表示
                gotogxy(5, 8);
                gprintf("TIME: %u", timecount);
            }

            if (count == 15){ //スコア表示
                gotogxy(5,11);
                gprintf("SCORE: %u", score);                
            }

            count++;
            
        }

    }
    // SELECT押されたらreset()
    while(1){
        if (joypad() & J_START){
            reset();
        }
    }


}