#ifndef _MARIO_GAME_STRUCT
#define _MARIO_GAME_STRUCT


#define BLOCK_NUM 45
#define MUSH_NUM 3
#define GOOMBA_NUM 10
#define CLOUD_NUM 7
#define GROUND_NUM 4
#define TUBE_NUM 5
#define COIN_NUM 25

#define GRAVITY (0.23)
#define MAX_SPEED_H (1.85)
#define MAX_SPEED_V (4.6)

// Mario data
#define WALK_ACC (0.09)
#define SHUT_ACC (0.12)
#define JUMP_INIT_V_SMALL (4.6)
#define JUMP_INIT_V_LARGE (5.6)
#define MAX_SPEED_V_JUMP (8.1)

// Mush DATA


// PPU CHILD LIMIT
#define BLOCK_VIS_LIMIT 9
#define COIN_VIS_LIMIT 4
#define GOOMBA_VIS_LIMIT 2
#define MUSH_VIS_LIMIT 1
#define TUBE_VIS_LIMIT 1
#define CLOUD_VIS_LIMIT 1

enum contact{NONE, LEFT, RIGHT, UP, DOWN};

typedef struct{
    float x, y, vx, vy; // location $ speed
    int sx, sy;         // size of the box
} hit_box;

typedef struct{
    unsigned int pattern_code;
    int visible, flip;
    int x, y;
    int extra_info;
} ppu_obj_info;

enum mario_control_state{MARIO_NORMAL, MARIO_ANIMATE};
enum mario_animate_state{HIT, ENLARGE, ANI_DEAD};
enum mario_game_state{MARIO_GAME_NORMAL, MARIO_GMAE_HIT};
enum mario_state{SMALL, LARGE, DEAD};

typedef struct {
    int enable;
	int x_block;
	int y_block;
    hit_box hit;
    ppu_obj_info vis;
    enum mario_control_state control_s;
    enum mario_game_state game_s;
    enum mario_state mario_s;
    enum mario_animate_state animate_s;
    float acc_x, acc_y; // Accelerate
    int animate_frame_counter;
} mario;

enum mush_state{MUSH_NORMAL, MUSH_ANIMATE};

typedef struct {
    int enable;
	int loaded;
	int x_block;
	int y_block;
    hit_box hit;
    ppu_obj_info vis;
    enum mush_state mush_s;
    float acc_x, acc_y; // Accelerate
    int animate_frame_counter;
} mush;

enum coin_state{COIN_NORMAL, COIN_ANIMATE};

typedef struct {
    int enable;
	int loaded;
    hit_box hit;
    ppu_obj_info vis;
    enum coin_state coin_s;
    int animate_frame_counter;
} coin;

enum goomba_state{GOOMBA_NORMAL, GOOMBA_ANIMATE};

typedef struct {
    int enable;
	int loaded;
	int x_block;
	int y_block;
    hit_box hit;
    ppu_obj_info vis;
    enum goomba_state goomba_s;
    float acc_x, acc_y; // Accelerate
    int r_limit, l_limit;
    int animate_frame_counter;
} goomba;

enum block_type{TYPE_A, TYPE_B_1, TYPE_B_2, TYPE_B_3, TYPE_B_4, TYPE_B_16, TYPE_A_H_8, OBJ_C, OBJ_M, EMP};
enum block_state{BLOCK_NORMAL, BLOCK_ANIMATE};

typedef struct {
    int enable;
	int loaded;
	int extra_info;
	int L;
	int R;
	int U;
	int D;
    hit_box hit;
    ppu_obj_info vis;
    enum block_type block_t;
    enum block_state block_s;
    int animate_frame_counter;
} block;

typedef struct {
    hit_box hit;
} ground;

typedef struct {
	int enable;
	int loaded;
    hit_box hit;
    ppu_obj_info vis;
} tube;

typedef struct {
    int enable;
	int loaded;
	hit_box hit;
    ppu_obj_info vis;
} cloud;

#define LOAD_LIMIT (5*16)
#define CAMERA_SIZE (30*16)
enum game_state{GAME_START, GAME_NORMAL, GAME_END};
typedef struct {
	enum game_state game_s;
	int camera_pos;
	mario mario_0;
	ground ground_list[GROUND_NUM];
	block block_list[BLOCK_NUM];
	coin coin_list[COIN_NUM];
    mush mush_list[MUSH_NUM];
    goomba goomba_list[GOOMBA_NUM];
    tube tube_list[TUBE_NUM];
    cloud cloud_list[CLOUD_NUM];
} mario_game;



enum contact hitbox_contact(const hit_box *A, const hit_box *B);
void new_game(mario_game *game);

#endif
