//http://blog.livedoor.jp/akf0/archives/51585502.html



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define CHOREN 12000 /* 長連 */
#define GOREN 10000 /* 五連 */
#define YONYON 880 /* 四四：２つの四連の一端が閉じていてもよい */
#define a_YONSAN 860 /* 三の両端が開いている四三 */
#define YONSAN 200 /* 四三：三の一端が閉じていてもよい */
#define a_SANSAN 750/* 両端が開いている三三が２つ以上 */
#define SANSAN 130 /* 三三：一端が閉じていてもよい */
#define a_YONREN 800 /* 両端が開いている四連 */
#define YONREN 70 /* 四連：一端が閉じていてもよい */
#define a_SANREN 150 /* 両端が開いた三連 */
#define SANREN 60 /* 三連：一端が閉じていてもよい */
#define NISAN 240 /* 両端が開いている二連三連*/
#define NIREN 40 /* 一端が相手石の二連 */
#define a_NIREN 80 /* 両端が開いた二連 */
#define ICHIREN 20 /* 一端が相手石の一連 */
#define a_ICHIREN 10 /* 両端が開いた一連 */ 
#define NINI 130
// #define b_SANREN 7 /* 両端が開いた飛び三 */
// #define b_YONREN  9 /* 両端が開いた飛び四*/ 
// #define c_SANREN 3 /* 一端が閉じた飛び三 */
// #define c_YONREN  5 /* 一端が閉じた飛び四*/ 



//display

#define BOARD_SQUARE 17

int board[BOARD_SQUARE][BOARD_SQUARE] = {{0}};
int a;




//宣言

int value = 0;  //評価値
int enemy_value = 0;
int max_value = 0; 
int value_a = 0;
int value_b = 0;
int max_tate = 0;
int max_yoko = 0;
int migi = 0;	//右に正
int ue = 0;		//上に正
int count = 0;	//自分の碁石の数
int cnt = 0;	//空白の数
int go = 0;
int yon = 0;	//碁石が四つあるかカウント
int num_cnt = 0;	//4回目まで見る用カウント
int sum_cnt = 0;	
int niren;          //二連の個数
int mn;				//三や四の数
int kuro = 0;		//六連確認用の自分の碁石の数
int rokuren = 0;	//六連チェック用
int guusuu = 0;		//〇◎〇の際に右を見る時と左を見る時にカウントに重複があるため、その数をカウント　偶数＝重複
int goren = 0;		//五連確認用
int challenge = 0;
int ban = 0;		//終了判定
int attack = 0;		//自分のターン
int counter = 0;	//相手のターン
int dx[5];			//深さ探索の時の仮に石を置く時用の配列のx座標
int dy[5];			//上記のy座標
int Min_count =0;
int sum_value = 0;
int eva_count = 0;	//探索の時に無駄なプログラムを実行しないための変数
int max_sum = 0;	//評価値の合計の最大値
int ent_cnt = 0;	//全体のカウント
int new_left,new_right;	//評価値さ荷台の場所が禁じ手だったときの新しいx,y座標
int ban_cnt = 0;	//◎　◎◎◎これの右から2番目に置くときに禁じ手にならないために用意した変数
int dif = 0;		//連の端に相手の石があるときなどの特殊な禁じ手を禁じ手と見なさないための変数
int di_cnt = 0;
int miss=0;

//関数宣言

void Checkin(int attack,int counter,int left,int right);
void hyouka(int attack,int counter);
int around(int attack,int counter,int yoko,int tate);
int renzoku(int attack,int counter,int yoko,int tate,int migi,int ue);
int reverse(int attack,int counter,int yoko,int tate,int migi,int ue);
int reset();
int loop(int left,int right);
int count1(int attack, int counter, int x, int y, int dx, int dy, int *more, int *type);
int count_more(int attack, int counter, int x, int y, int dx, int dy, int *type);
int line(int attack, int counter, int x, int y, int dx, int dy);
int pattern(int x,int y,int attack, int counter);
void  Min_evaluate(int attack, int counter, int max_x, int max_y);



int main(void) {	

	for(a = 0; a < 17; a++){
		board[0][a] = -1;
		board[a][0] = -1;	//盤外
		board[16][a] = -1;
		board[a][16] = -1;
	}

	//文字入力用

	int left;
	int right;
	int bla_cnt = 0;	//自分が先行だった時データ受け取りカウント

	//接続するサーバの情報の構造体を用意

	struct sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));

	printf("アドレス番号入力してください\n");
	char destination [256];  //アドレス
	scanf("%s",destination);

	//strcpy(destination,"127.0.0.1");
	printf("black or white?\n");

	//ポート番号選択
	char str[256];
	scanf("%s",str);
	if(strcmp(str,"black") == 0){

		int poto = 12345;
		dest.sin_port = htons(poto);  //ポート番号

	}else{
		int poto = 12346;
		dest.sin_port = htons(poto);  //ポート番号
	}

	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(destination);

	//ソケット通信の準備・生成

	WSADATA data;
	WSAStartup(MAKEWORD(2, 0), &data);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	//サーバへの接続

	if (connect(s, (struct sockaddr *) &dest, sizeof(dest))) {
		
		printf("%sに接続できませんでした\n", destination);
		return -1;
	}
	printf("%sに接続しました\n", destination);

	//サーバからデータを受信

		char buffer[1024];
	recv(s, buffer, 1024, 0);
	printf("→ %s\n", buffer);
	char msg [256];

	printf("send = Katofuya\n");
	strcpy(msg,"Kato-Fuya");

	//サーバにデータを送信
	send(s, msg, strlen(msg), 0);


	while(1){

		ent_cnt++;
		bla_cnt++;

		while(-1 == recv(s, buffer, 1024, 0)){}



		if(strcmp(str,"black") == 0 && bla_cnt == 1){

			char ms[256];
			strcpy(ms,"8,8");
			board[8][8] = 1;
			send(s, ms, strlen(ms), 0);

		}else if(strcmp(str,"black") == 0 && bla_cnt > 1){

			attack = 2;
			counter = 1;

            if(strcmp(buffer,"win") == 0){
                char msg[256];
				sprintf(msg,"%s","end");
				send(s, msg, strlen(msg), 0);
            }else if(strcmp(buffer,"forbidden") == 0){
                char msg[256];
				sprintf(msg,"%s","end");
				send(s, msg, strlen(msg), 0);
            }else if(strcmp(buffer,"end") == 0){
                break;
            }else if(strcmp(buffer,"draw") == 0){
				break;
			}

			//printf(buffer);

			char *num, *num2;
			num = strtok(buffer,",");
			left = atoi(num);
			num2 = strtok(NULL,",");
			right = atoi(num2);
			board[left][right] = 2;

			//相手の禁じ手かチェック

			//printf("%d %d",left,right);



			//自分が１で、相手が２の時

			attack = 1;
			counter = 2;

			//printf("サーバーに入力する文字列を決めてください。\n");

			//scanf("%s",po2);		//試作

			hyouka(attack,counter);
			max_sum = 0;
			//printf("%d %d %d",max_value,max_tate,max_yoko);


			// left = max_yoko;
			// right = max_tate;
			//自分が禁じ手かどうか確認

			left = max_yoko;
			right = max_tate;

		
			Checkin(attack,counter,left,right);
			
			if(challenge == 1){
				challenge = 0;
				loop(left,right);
                left = new_left;
                right = new_right;
			}

			if(pattern(left,right,attack,counter) == GOREN){
				char msg[256];
				printf("五連です。プレイヤーの勝利\n");
				sprintf(msg,"%d,%d,%s",left,right,"win");
				send(s, msg, strlen(msg), 0);
			}

			char msg[256];
			sprintf(msg,"%d,%d",left,right);

			send(s, msg, strlen(msg), 0);

			int h,b;
			for(h=1;h<=15;h++){
				for(b=1;b<=15;b++){
					printf("%d ",board[b][h]);
				}printf("\n");
			}

			printf("send(left %d right %d)\n\n",left,right);

			printf("----------------------------------------\n");
			int g,l;
			int cnt = 0;
			for(g = 1;g <= 15;g++){
				for(l = 1;l <= 15;l++){
					if(board[l][g] != 0){
						cnt++;
					}
				}
			}
			if(cnt == 225){
				char msg[256];
				sprintf(msg,"%s","draw");
				send(s, msg, strlen(msg), 0);
			}

		}else if(strcmp(str,"white") == 0){

			attack = 2;

			counter = 1;

			if(strcmp(buffer,"win") == 0){
                char msg[256];
				sprintf(msg,"%s","end");
				send(s, msg, strlen(msg), 0);
            }else if(strcmp(buffer,"forbidden") == 0){
                char msg[256];
				sprintf(msg,"%s","end");
				send(s, msg, strlen(msg), 0);
            }else if(strcmp(buffer,"end") == 0){
                break;
            }else if(strcmp(buffer,"draw") == 0){
				break;
			}

			char *num, *num2;
			num = strtok(buffer,",");
			left = atoi(num);
			num2 = strtok(NULL,",");
			right = atoi(num2);
			board[left][right] = 2;

			//相手の禁じ手かチェック

			Checkin(attack,counter,left,right);

			if(ban == 1){
				char msg[256];
				strcpy(msg,"forbidden");
				send(s, msg, strlen(msg), 0);
			}



			//自分が１で、相手が２の時

			attack = 1;
			counter = 2;

			//printf("サーバーに入力する文字列を決めてください。\n");
			hyouka(attack,counter);
			max_sum = 0;
			//printf("%d %d %d",max_value,max_tate,max_yoko);

			//受け取った値をint化し、配列boardに入れる

			left = max_yoko;
			right = max_tate;

			board[left][right] = 1;
			int check = around(attack,counter,left,right);
			if(pattern(left,right,attack,counter) == GOREN || pattern(left,right,attack,counter) == CHOREN){
				char msg[256];
				printf("五連です。プレイヤーの勝利\n");
				sprintf(msg,"%d,%d,%s",left,right,"win");
				send(s, msg, strlen(msg), 0);
			}
			char msg[256];
			sprintf(msg,"%d,%d",left,right);
			send(s, msg, strlen(msg), 0);

			int h,b;
			for(h=1;h<=15;h++){
				for(b=1;b<=15;b++){
					printf("%d ",board[b][h]);
				}printf("\n");
			}printf("\n");

			printf("send(left %d right %d)\n\n",left,right);
			printf("----------------------------------------\n");
			int g,l;
			int cnt = 0;
			for(g = 1;g <= 15;g++){
				for(l = 1;l <= 15;l++){
					if(board[l][g] != 0){
						cnt++;
					}
				}
			}
			if(cnt == 225){
				char msg[256];
				sprintf(msg,"%s","draw");
				send(s, msg, strlen(msg), 0);
			}
		}
	//サーバからデータを受信
	// char buffer1[1024];
	// recv(s, buffer1, 1024, 0);
	}
	// Windows でのソケットの終了
	closesocket(s);
	WSACleanup();
	return 0;
}




int loop(int left,int right){
	printf("禁じ手だったのでループします\n");
	board[left][right] = 3;
	hyouka(attack,counter);
	max_sum = 0;
    new_left = max_yoko;
    new_right = max_tate;
	Checkin(attack,counter,new_left,new_right);
	if(challenge == 1){
		challenge = 0;
		return loop(new_left,new_right);
	}else{
		int m,n;
		for(m = 1;m <= 15; m++){
			for(n = 1; n <= 15; n++){
				if(board[m][n] == 3){
					board[m][n] = 0;
				}
			}
		}
		return 0;
	}
}




void Checkin(int attack,int counter,int left,int right){

	sum_cnt = 0;
	int check = around(attack,counter,left,right);

	if(attack == 1){

		//禁じ手じゃなければ

		if(check == 0){
			//printf("%d , %d , %d , %d",mn, yon, kuro, rokuren);
			board[left][right] = 1;		
			mn = 0;
			yon = 0;
			guusuu = 0;
		}else if(check == 2 || pattern(left,right,attack,counter) == GOREN){
			board[left][right] = 1;
			ban++;
			printf("五連です。プレイヤーの勝利\n");

		}else{		//禁じ手発見
			//printf("%d , %d , %d , %d",mn, yon, kuro, rokuren);
			challenge = 1;	
		}
	}else{

		//禁じ手じゃなければ

		if(check == 0){	
			mn = 0;
			yon = 0;
			guusuu = 0;
		}else if(check == 2){
			printf("五連です。相手の勝利\n");
		}else{		//禁じ手発見
			printf("相手の負け.ゲームを終了します\n");
			ban++;
		}
	}
}




int around(int attack,int counter,int yoko,int tate) {

	mn = 0;		//禁じ手判断

	//int final = 0;	//最終禁じ手あるかどうか

	//右一直線

	//board[tate][yoko + 1] == 1

		niren = 0;
		mn = 0;
		yon = 0;
		go = 0;
		goren = 0;
		value = 0;
		guusuu = 0;
		count = 0;
		dif = 0;
		di_cnt = 0;
		miss = 0;


		//右
		ue = 0;
		migi = 1;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		ban_cnt = 0;
		int my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = 0;
			migi = -1;
			reverse(attack,counter,yoko,tate,migi,ue);
		}

		//左

		//board[tate][yoko - 1] == 1

		ue = 0;
		migi = -1;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;

		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = 0;
			migi = 1;
			reverse(attack,counter,yoko,tate,migi,ue);
		}
		ban_cnt = 0;
		di_cnt = 0;

	//右上
	//board[tate + 1][yoko + 1] == 1
		ue = 1;
		migi = 1;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = -1;
			migi = -1;
			reverse(attack,counter,yoko,tate,migi,ue);
		}
		
	//左下

	//board[tate - 1][yoko - 1] == 1

		ue = -1;
		migi = -1;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = 1;
			migi = 1;
			reverse(attack,counter,yoko,tate,migi,ue);
		}
		ban_cnt = 0;
		di_cnt = 0;


	//右下

	//board[tate - 1][yoko + 1] == 1
		ue = -1;
		migi = 1;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = 1;
			migi = -1;
			reverse(attack,counter,yoko,tate,migi,ue);
		}

		//左上

	//board[tate + 1][yoko - 1] == 1
		ue = 1;
		migi = -1;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = -1;
			migi = 1;
			reverse(attack,counter,yoko,tate,migi,ue);
		}
		ban_cnt = 0;
		di_cnt = 0;


		//上

	//board[tate + 1][yoko] == 1
		ue = 1;
		migi = 0;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);
		if(my_serial != 0 && my_serial != 3){
			ue = -1;
			migi = 0;
			reverse(attack,counter,yoko,tate,migi,ue);
		}

	//下

	//board[tate - 1][yoko] == 1
		ue = -1;
		migi = 0;
		count = 0;		//count初期化
		go = 0;
		kuro = 0;
		my_serial = renzoku(attack,counter,yoko,tate,migi,ue);

		if(my_serial != 0 && my_serial != 3){
			ue = 1;
			migi = 0;
			reverse(attack,counter,yoko,tate,migi,ue);
		}
		if(miss == 1){
			dif++;
		}

		//確認
	if(goren == 1){
		return 2;
	}else if(yon == 2){
		printf("四四禁です\n");
		return 1;
	}else if(rokuren == 1 && goren == 0 && dif== 0){
		printf("六連です\n");
		return 1;
	}else if(mn >= 2 && yon < 1 && dif == 0){
		printf("三三禁です\n");
		return 1;
	}else{
		return 0;
	}
}



void hyouka(int attack, int counter){
	int i,j,k,l;
	max_value = 0;
	max_yoko = 0;
	max_tate = 0;
    for(i = 1; i <= 15; i++){
		for(j = 1; j <= 15; j++){
			if(board[i][j] == 0){
				
				if(pattern(i,j,counter,attack) == a_YONREN){
					value = a_YONREN-300;
				}
				if(pattern(i,j,counter,attack) == a_SANSAN){
					value = a_SANSAN-300;
				}
				if(pattern(i,j,counter,attack) == a_YONSAN){
					value = a_YONSAN-300;
				}
				if(pattern(i,j,counter,attack) == YONYON){
					value = YONYON-300;
				}
				if(pattern(i,j,attack,counter) == a_YONREN){
					printf("四連\n");
					value = a_YONREN;
				}
				if(pattern(i,j,attack,counter) == a_SANSAN){
					printf("三三\n");
					value = a_SANSAN;
				}
				if(pattern(i,j,attack,counter) == a_YONSAN){
					printf("四三\n");
					value = a_YONSAN;
				}
				if(pattern(i,j,attack,counter) == YONYON){
					printf("四四\n");
					value = YONYON;
				}
				if(pattern(i,j,counter,attack) == CHOREN || pattern(i,j,counter,attack) == GOREN){
					printf("五連阻止\n");
					value = GOREN-1000;
				}
				if(pattern(i,j,attack,counter) == CHOREN || pattern(i,j,attack,counter) == GOREN){
					printf("五連\n");
					value = GOREN;
				}
				if(value > 0 && value >= max_value){
					max_value = value;
					value = 0;
					max_tate = j;
					max_yoko = i;
					// printf("1座標（%d,%d）",i,j);
					// printf("自分パターン:%d\t",pattern(i,j,attack,counter));
					// printf("相手パターン:%d\n",pattern(i,j,counter,attack));
					printf("value=%d",max_value);
				}
			}
		}
	}
	if(max_tate==0&&max_yoko==0){
		for(k = 1; k <= 15; k++){
			for(l = 1; l <= 15; l++){
				if(board[k][l] == 0){
					
					value_a = pattern(k,l,attack,counter);
					value_b = pattern(k,l,counter,attack)*0.8;
					value = value_a + value_b;
					if(value > max_value){
						max_value = value;
						max_tate = l;
						max_yoko = k;
					}
				}
				// 	if(value>18){
				// 		printf("自分パターン:%d\n",pattern(k,l,attack,counter));
				// 		printf("相手パターン:%d\n",pattern(k,l,counter,attack));
				// 		printf("座標(%d,%d) 評価値 %d + %d = %d\n",k,l,value_a,value_b,value);
				// 	}
				// }else if(board[k][l]==2){
				// 	printf("相手の座標（%d,%d）\n",k,l);
				// }else if(board[k][l]==1){
				// 	printf("自分の座標（%d,%d）\n",k,l);
				// }
			}
		}
	}
	printf("座標(%d,%d) 評価値最大%d\n",max_yoko,max_tate,max_value);
}




//正規ルート探索

int renzoku(int attack,int counter, int yoko,int tate,int migi,int ue){
	sum_cnt++;		//全体のカウント
	if(1 <= tate + ue && tate + ue <= 15 && 1 <= yoko + migi && yoko + migi <= 15){
		//printf("tate yoko(%d %d)",tate+ue,yoko + migi);
		
		if(sum_cnt ==5){	//〇　◎◎◎　の時などに入る
			if(kuro == 4){
				goren = 1;
			}
			if(board[yoko + migi][tate + ue] == attack && kuro == 5){
				kuro++;
			}
			count = 0;
			cnt = 0;
			sum_cnt=0;
			return 1;
		}

		if(board[yoko + migi][tate + ue] == attack){
			count++;
			tate = tate + ue;
			yoko = yoko + migi;

			if(count == 1){
				niren++;
			}
			if(count == 2){			//三連ができていたらカウント
				mn++;
				ban_cnt++;
			}else if(count == 3){	//四連ができていたらカウント
				yon++;
			}else if(count == 4){
				go++;
			}
			if(go == 4){
				goren = 1;
			}

			if(count == sum_cnt){	//碁石が連続で並んでいたらカウント
				kuro++;
			}

			if(board[yoko + migi][tate + ue] == counter){
				if(count == 2 || count == 1){
					dif++;
				}
			}
			//printf("1%d",dif);
			return renzoku(attack,counter,yoko,tate,migi,ue);

		}else if(board[yoko + migi][tate + ue] == 0){		//空白だった時

			cnt++;
			tate = tate + ue;
			yoko = yoko + migi;
			if(cnt == 2){
				cnt = 0;
				count = 0;
				
				if(sum_cnt == 2){		//2個連続で空白だった場合
					sum_cnt = 0;
					return 0;	
				}else{
					//printf("%d %d\n",sum_cnt, niren);
					sum_cnt = 0;
					return 2;
				}
			}else{
				return renzoku(attack,counter,yoko,tate,migi,ue);
			}

		}else if(board[yoko + migi][tate + ue] == counter){
			if(count > 0 && cnt > 0){
				di_cnt++;
			}
			if(cnt == 1){
				sum_cnt = 0;
				cnt = 0;
				count=0;
				return 2;
			}else{
				sum_cnt = 0;
				cnt = 0;
				count = 0;
				return 3;
			}
		}
	}else{
		sum_cnt = 0;
		cnt = 0;
		count = 0;
		return 3;
	}
}




//逆方向探索

int reverse(int attack, int counter, int yoko,int tate,int migi,int ue){

	if(1 <= tate + ue && tate + ue <= 15 && 1 <= yoko + migi && yoko + migi <= 15){

		if(board[yoko + migi][tate + ue] == attack){
			guusuu++;
			count++;
			tate = tate + ue;
			yoko = yoko + migi;
			mn++;
			kuro++;
			if(ban_cnt == 1){
				miss++;
			}
			if(kuro >= 5){		//六連ならば
				rokuren = 1;
			}else if(kuro == 4){
				goren = 1;
			}
			if(guusuu % 2 == 0){
				mn--;
				kuro--;
			}
			if(ban_cnt % 2 == 1){
				mn--;
				mn--;
			}
			if(count == 2){
				yon++;
				mn--;
				return 1;
			}else{
				return reverse(attack,counter,yoko,tate,migi,ue);
			}
		}else if(count == 0){
			if(kuro >= 5){
				rokuren = 1;
			}else if(kuro == 4){
				goren = 1;
			}
			return 0;

		}else if(board[yoko + migi][tate + ue] == counter){

			if(count >= 1){
				dif++;
			}
			//printf("2%d",dif);

			return 3;

		}else if(board[yoko + migi][tate + ue] == 0){
			if(board[yoko + migi + migi][tate + ue + ue] == counter){
				if(count >= 1){
					dif++;
				}
			//printf("3%d",dif);

				di_cnt++;
				if(di_cnt == 2){
					dif++;
				}
			//printf("4%d",dif);
			}
			return 4;
		}
	}
}

int count1(int attack, int counter, int x, int y, int dx, int dy, int *more, int *type){	//石が連なっている個数
	
	if(board[x][y] == attack){
		return 1 + count1(attack, counter, x + dx, y + dy, dx, dy, more,type);
	}else if(board[x][y] == -1){
		*more = 0;
		*type = 0;		
		return 0;
	}else if(board[x][y] == counter){
		*more = 0;
		*type = 1;		
		return 0;
	}else if(board[x][y] == 0){
		*more = count_more(attack, counter, x + dx, y + dy, dx, dy,type);
		return 0;
	}
}
int count_more(int attack, int counter, int x, int y, int dx, int dy, int *type){	//空白の後に連なっている個数
	
	if(board[x][y] == attack){
		return 1 + count_more(attack, counter, x + dx, y + dy, dx ,dy,type);
	}else if(board[x][y] == -1){
		*type = 2;	//盤外の時
		return 0;
	}else if(board[x][y] == counter){
		*type = 3;	
		return 0;
	}else if(board[x][y] == 0){
		if(board[x+dx+dx][y+dy+dy]==0)
		*type = 6; 		//4 個連続の空白あり
		else *type = 5; 	//この場合、3 個連続の空白あり
		}
		else *type = 4; 	//この場合、2 個連続の空白あり
		return 0; 
}

int line(int attack, int counter, int x, int y, int dx, int dy){
	int cnt1,cnt_a,cnt_b,more,more_a,more_b,type_a,type_b;
	// if(x<=0||x>=16||y<=0||y>=16||board[x][y]!=0){
	// 	return 0;
	// }

	cnt_a = count1(attack, counter, x + dx, y + dy, dx, dy, &more_a, &type_a);
	cnt_b = count1(attack, counter, x - dx, y - dy, -dx, -dy, &more_b, &type_b);
	cnt1 = 1 + cnt_a + cnt_b; 
	more = more_a + more_b;
	if (cnt1>=6) return CHOREN; /* 長連 */
	if (cnt1==5) return GOREN; /* 五連 */
	if(cnt1==4) {
		if(type_a>=2 && type_b>=2)
		return a_YONREN;
		if(type_a>=2 || type_b>=2)
		return YONREN;
	}

	if(cnt1==3) {
		if(more>0) return YONREN;
		if(type_a>=2 && type_b>=2)
		return a_SANREN;
		if(type_a>=2 || type_b>=2)
		return SANREN;
	}
	if(cnt1==2) {
		if(more_a>=3 || more_b>=3)
		return YONREN;
		if(more>0 && type_a>=2 && type_b>=2)
		return SANREN;
		if(type_a>=2 && type_b>=4 || type_a>=4 && type_b>=2)
		return a_NIREN;
		if(type_a==1 && type_b==5 || type_a==5 && type_b==1)
		return NIREN;
	}
	if(cnt1==1) {
		if(more_a>=3 || more_b>=3)
		return YONREN;
		if(more_a>=2 && (type_a>=4 || type_b>2) ||
		more_b>=2 && (type_b>=4 || type_a>2))
		return SANREN;
		if(type_a>=2 && type_b>=5 || type_a>=5 && type_b>=2)
		return a_ICHIREN;
		if(type_a==1 && type_b>=6 || type_b==1 && type_a>=6)
		return ICHIREN; 
	}
}

int pattern(int x,int y,int attack, int counter){
	int yon,yon1,yon2,san,san1,san2,line1,line2,line3,line4;
	line1 = line(attack, counter, x, y, 1, 0);
	line2 = line(attack, counter, x, y, 1, 1);
	line3 = line(attack, counter, x, y, 0, 1);
	line4 = line(attack, counter, x, y, -1, 1);

	// if(line1==0||line2==0||line3==0||line4==0){
	// 	return 0;
	// }
	
	if(line1==CHOREN || line2==CHOREN || line3==CHOREN || line4==CHOREN)
	return CHOREN;
	if(line1==GOREN || line2==GOREN || line3==GOREN || line4==GOREN)
	return GOREN;
	yon1 = (line1==YONREN)+(line2==YONREN)+(line3==YONREN)+(line4==YONREN);
	yon2 = (line1==a_YONREN)+(line2==a_YONREN)
	+(line3==a_YONREN)+(line4==a_YONREN);
	if((yon=yon1+yon2)>=2) return YONYON;
	san2 = (line1==a_SANREN)+(line2==a_SANREN)
	+(line3==a_SANREN)+(line4==a_SANREN);
	san1 = (line1==SANREN)+(line2==SANREN)
	+(line3==SANREN)+(line4==SANREN);
	if(yon==1) {
		if(san2>=1) return a_YONSAN;
		if(yon2>=1) return a_YONREN;
		if(san1>=1) return YONSAN;
	}
	
	if(san2>=2) return a_SANSAN;
	if(san2==1&&((line1==a_NIREN)+(line2==a_NIREN)
	+(line3==a_NIREN)+(line4==a_NIREN))>=1) return NISAN;
	if(san2+san1>=2) return SANSAN;
	if(san2==1) return a_SANREN;
	if(san1==1) return SANREN;
	if((line1==NIREN)+(line2==NIREN) +(line3==NIREN)+(line4==NIREN)>0)
	return NIREN;
	if((line1==a_NIREN)+(line2==a_NIREN)
	+(line3==a_NIREN)+(line4==a_NIREN)>=2)
		return NINI;
	if((line1==a_NIREN)+(line2==a_NIREN)
	+(line3==a_NIREN)+(line4==a_NIREN)>0)
		return a_NIREN;
	if((line1==ICHIREN)+(line2==ICHIREN)
	+(line3==ICHIREN)+(line4==ICHIREN)>0)
		return ICHIREN;
	if((line1==a_ICHIREN)+(line2==a_ICHIREN)
	+(line3==a_ICHIREN)+(line4==a_ICHIREN)>0) return a_ICHIREN;

	return 0;
}