#define VRAM_BASE_B 0xA000
#define TILE_HEIGHT_B 8
#define TILE_WIDTH_B 8

/*
 * origin (0/0) of pixel coordinates is top left corner of the screen
 * ball coordinates are top left corner of a rectangle around the ball
 * so ball coordinates (0/0) mean the ball is in the top left corner of the screen
 */
#define WALL_LEFT 0
#define WALL_RIGHT 350
#define WALL_BOTTOM 200
#define WALL_TOP 0

/* these values will be transferred to Copper from assembly code */
int dst; /* position in VRAM */
int scroll; /* pixel based x/y-offset */
int gfxctrl; /* palette choice */

static int colour;
static int v_x;
static int v_y;
static int a_y;
static int top_left_x;
static int top_left_y;

void init_physics(void)
{
	colour = 0; /* initial palette */

	v_x = 7; /* horizontal speed */
	top_left_x = WALL_LEFT; /* horizontal start coordinate */

	v_y = 0; /* initial vertical speed */
	a_y = 5; /* vertical acceleration scaled by 10, so this is really 0.5 */
	top_left_y = WALL_TOP; /* vertical start coordinate */
}

/* physics engine is called once every frame */
void do_physics(void)
{
	int top_left_row;
	int top_left_col;
	int scroll_x;
	int scroll_y;

	/* vertical speed is scaled by 10 to improve accuracy */
	v_y += a_y;
	top_left_y += (v_y / 10);
	if ((top_left_y > WALL_BOTTOM) || (top_left_y < WALL_TOP)) {
		v_y = -v_y;
		top_left_y += (v_y / 10); /* conservation of momentum */
	}

	/* horizontal speed is constant */
	top_left_x += v_x;
	if ((top_left_x > WALL_RIGHT) || (top_left_x < WALL_LEFT))
		v_x = -v_x;

	/*
	 * physics done, prepare register values for Copper now
	 */

	/* make tile coordinates from pixel coordinates */
	top_left_row = (top_left_y + TILE_HEIGHT_B - 1) / TILE_WIDTH_B;
	top_left_col = (top_left_x + TILE_WIDTH_B - 1) / TILE_WIDTH_B;

	/* make VRAM address from tile based coordinates where Copper will blit the ball */
	dst = VRAM_BASE_B + top_left_row * (640 / TILE_WIDTH_B) + top_left_col;

	/* shift tile plane to reach pixel accuracy */
	scroll_x = -(top_left_x - top_left_col * TILE_WIDTH_B);
	scroll_y = -(top_left_y - top_left_row * TILE_HEIGHT_B);

	/* value will be written to PB_HV_SCROLL by Copper */
	scroll = (scroll_x << 8) | (scroll_y << 2);

	/* rotate palettes to rotate ball */
	if (v_x < 0) {
		colour += 0x1000;
		if (colour == 0xE000)
			colour = 0;
	} else {
		if (colour == 0x0000) {
			colour = 0xD000;
		} else {
			colour -= 0x1000;
			if (colour == 0xE000)
				colour = 0;
		}
	}
	/* value will be written to PB_GFX_CTRL by Copper */
	gfxctrl = colour | (1<<4);
}
