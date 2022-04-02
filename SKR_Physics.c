#include "SKR_Physics.h"
#include <stdlib.h>
#include <math.h>

typedef struct {
	float x;
	float y;
}SKR_Vec2;

struct SKR_StaticRect {
	SKR_Rect* position;
	SKR_StaticRect* sonraki;
};

struct SKR_KinematicRect {
	SKR_Rect* position;
	SKR_Vec2 velocity;
	SKR_KinematicRect* sonraki;
	int boolean;
};

struct SKR_DynamicRect {
	SKR_Rect* position;
	float mass;
	SKR_Vec2 velocity;
	float xk;
	float yk;
	SKR_Vec2 force;
	SKR_DynamicRect* sonraki;
	int isonground;
	float friction;
	int alt, ust, sol, sag;
	float gravitymultiplier;
};

struct SKR_RectWorld {
	float gravity;
	float airfriction;
	SKR_GAMETYPE gametype;
	SKR_StaticRect* StaticRectList;
	SKR_KinematicRect* KinematicRectList;
	SKR_DynamicRect* DynamicRectList;
};

float* xmax = NULL, * xmin = NULL, * ymax = NULL, * ymin = NULL;

float Xmax, Ymax;

float xmax2, ymax2, xmin2;

int loop, i;

SKR_Vec2 a, b, c, d;

SKR_DynamicRect* tmpD;
SKR_DynamicRect* tmpD2;
SKR_StaticRect* tmpS;
SKR_KinematicRect* tmpK;

int onSegment(SKR_Vec2 p, SKR_Vec2 q, SKR_Vec2 r) {

	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) {
		return 1;
	}

	return 0;
}

int orientation(SKR_Vec2 p, SKR_Vec2 q, SKR_Vec2 r) {

	if (((q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y)) == 0) {
		return 0;
	}

	return (((q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y)) > 0) ? 1 : 2;

}

int doIntersect(SKR_Vec2 p1, SKR_Vec2 q1, SKR_Vec2 p2, SKR_Vec2 q2) {

	if (orientation(p1, q1, p2) != orientation(p1, q1, q2) && orientation(p2, q2, p1) != orientation(p2, q2, q1)) { return 1; }

	if (orientation(p1, q1, p2) == 0 && onSegment(p1, p2, q1)) { return 1; }

	if (orientation(p1, q1, q2) == 0 && onSegment(p1, q2, q1)) { return 1; }

	if (orientation(p2, q2, p1) == 0 && onSegment(p2, p1, q2)) { return 1; }

	if (orientation(p2, q2, q1) == 0 && onSegment(p2, q1, q2)) { return 1; }

	return 0;
}

int SKR_IntersectRectLine(SKR_Rect* Rect, float* x1, float* y1, float* x2, float* y2) {
	a.x = *x1;
	a.y = *y1;
	b.x = *x2;
	b.y = *y2;
	c.x = Rect->x;
	c.y = Rect->y;
	d.x = Rect->x + Rect->w;
	d.y = Rect->y;
	if (doIntersect(a, b, c, d)) {
		return 1;
	}
	d.x = Rect->x;
	d.y = Rect->y + Rect->h;
	if (doIntersect(a, b, c, d)) {
		return 1;
	}
	c.x = Rect->x + Rect->w;
	c.y = Rect->y + Rect->h;
	if (doIntersect(a, b, c, d)) {
		return 1;
	}
	d.x = Rect->x + Rect->w;
	d.y = Rect->y;
	if (doIntersect(a, b, c, d)) {
		return 1;
	}
	if (Rect->x <= min(*x1, *x2) && (Rect->x + Rect->w) >= max(*x1, *x2) && Rect->y <= min(*y1, *y2) && (Rect->y + Rect->h) >= max(*y1, *y2)) {
		return 1;
	}
	return 0;
}

int SKR_IntersectRectRect(SKR_Rect* Rect1, SKR_Rect* Rect2) {
	return (Rect1->x <= (Rect2->x + Rect2->w) && Rect2->x <= (Rect1->x + Rect1->w) && Rect1->y <= (Rect2->y + Rect2->h) && Rect2->y <= (Rect1->y + Rect1->h));
}

void CollideDynamicStatic(SKR_DynamicRect* DynamicRect, SKR_StaticRect* StaticRect) {
	if (SKR_IntersectRectRect(DynamicRect->position, StaticRect->position)) {
		*xmin = StaticRect->position->x;
		*ymin = StaticRect->position->y;
		*xmax = StaticRect->position->x + StaticRect->position->w;
		*ymax = StaticRect->position->y + StaticRect->position->h;
		if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = StaticRect->position->y - DynamicRect->position->h - 0.01f;
			if (DynamicRect->velocity.y > 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->isonground = 1;
			DynamicRect->alt = 1;
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = StaticRect->position->x + StaticRect->position->w + 0.01f;
			if (DynamicRect->velocity.x < 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sol = 1;
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = StaticRect->position->y + StaticRect->position->h + 0.01f;
			if (DynamicRect->velocity.y < 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->ust = 1;
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = StaticRect->position->x - DynamicRect->position->w - 0.01f;
			if (DynamicRect->velocity.x > 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sag = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((StaticRect->position->x + StaticRect->position->w - DynamicRect->position->x) >= ((DynamicRect->position->y + DynamicRect->position->h - StaticRect->position->y))) {
				DynamicRect->position->y = StaticRect->position->y - DynamicRect->position->h - 0.01f;
				if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->isonground = 1;
				DynamicRect->alt = 1;
			}
			else {
				DynamicRect->position->x = StaticRect->position->x + StaticRect->position->w + 0.01f;
				if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sol = 1;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((StaticRect->position->x + StaticRect->position->w - DynamicRect->position->x) >= ((StaticRect->position->y + StaticRect->position->h - DynamicRect->position->y) + DynamicRect->velocity.y)) {
				DynamicRect->position->y = StaticRect->position->y + StaticRect->position->h + 0.01f;
				if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->ust = 1;
			}
			else {
				DynamicRect->position->x = StaticRect->position->x + StaticRect->position->w + 0.01f;
				if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sol = 1;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect->position->x + DynamicRect->position->w - StaticRect->position->x) >= ((StaticRect->position->y + StaticRect->position->h - DynamicRect->position->y) + DynamicRect->velocity.y)) {
				DynamicRect->position->y = StaticRect->position->y + StaticRect->position->h + 0.01f;
				if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->ust = 1;
			}
			else {
				DynamicRect->position->x = StaticRect->position->x - DynamicRect->position->w - 0.01f;
				if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sag = 1;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect->position->x + DynamicRect->position->w - StaticRect->position->x) >= ((DynamicRect->position->y + DynamicRect->position->h - StaticRect->position->y))) {
				DynamicRect->position->y = StaticRect->position->y - DynamicRect->position->h - 0.01f;
				if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->isonground = 1;
				DynamicRect->alt = 1;
			}
			else {
				DynamicRect->position->x = StaticRect->position->x - DynamicRect->position->w - 0.01f;
				if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sag = 1;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = StaticRect->position->y + StaticRect->position->h + 0.01f;
			if (DynamicRect->velocity.y < 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->ust = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = StaticRect->position->x - DynamicRect->position->w - 0.01f;
			if (DynamicRect->velocity.x > 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sag = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = StaticRect->position->y - DynamicRect->position->h - 0.01f;
			if (DynamicRect->velocity.y > 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->isonground = 1;
			DynamicRect->alt = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = StaticRect->position->x + StaticRect->position->w + 0.01f;
			if (DynamicRect->velocity.x < 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sol = 1;
		}
	}
	return;
}

void CollideDynamicKinematic(SKR_DynamicRect* DynamicRect, SKR_KinematicRect* KinematicRect, float gravity, SKR_GAMETYPE gametype) {
	if (SKR_IntersectRectRect(DynamicRect->position, KinematicRect->position)) {
		*xmin = KinematicRect->position->x;
		*ymin = KinematicRect->position->y;
		*xmax = KinematicRect->position->x + KinematicRect->position->w;
		*ymax = KinematicRect->position->y + KinematicRect->position->h;
		if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = KinematicRect->position->y - DynamicRect->position->h - 0.01f;
			if (DynamicRect->velocity.y > 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->alt = 1;
			if (gametype == SKR_SIDESCROLLER) {
				DynamicRect->isonground = 1;
				DynamicRect->position->y += 0.02f;
				if (KinematicRect->velocity.y > 0 && KinematicRect->velocity.y <= gravity) {
					DynamicRect->yk = KinematicRect->velocity.y;
				}
				DynamicRect->xk = KinematicRect->velocity.x;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = KinematicRect->position->x + KinematicRect->position->w + 0.01f;
			if (DynamicRect->velocity.x < 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sol = 1;
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = KinematicRect->position->y + KinematicRect->position->h + 0.01f;
			if (DynamicRect->velocity.y < 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->ust = 1;
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = KinematicRect->position->x - DynamicRect->position->w - 0.01f;
			if (DynamicRect->velocity.x > 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sag = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((KinematicRect->position->x + KinematicRect->position->w - DynamicRect->position->x) >= ((DynamicRect->position->y + DynamicRect->position->h - KinematicRect->position->y))) {
				DynamicRect->position->y = KinematicRect->position->y - DynamicRect->position->h - 0.01f;
				if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->alt = 1;
				if (gametype == SKR_SIDESCROLLER) {
					DynamicRect->isonground = 1;
					DynamicRect->position->y += 0.02f;
					if (KinematicRect->velocity.y > 0 && KinematicRect->velocity.y <= gravity) {
						DynamicRect->yk = KinematicRect->velocity.y;
					}
					DynamicRect->xk = KinematicRect->velocity.x;
				}
			}
			else {
				DynamicRect->position->x = KinematicRect->position->x + KinematicRect->position->w + 0.01f;
				if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sol = 1;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((KinematicRect->position->x + KinematicRect->position->w - DynamicRect->position->x) >= ((KinematicRect->position->y + KinematicRect->position->h - DynamicRect->position->y) + DynamicRect->velocity.y)) {
				DynamicRect->position->y = KinematicRect->position->y + KinematicRect->position->h + 0.01f;
				if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->ust = 1;
			}
			else {
				DynamicRect->position->x = KinematicRect->position->x + KinematicRect->position->w + 0.01f;
				if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sol = 1;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect->position->x + DynamicRect->position->w - KinematicRect->position->x) >= ((KinematicRect->position->y + KinematicRect->position->h - DynamicRect->position->y) + DynamicRect->velocity.y)) {
				DynamicRect->position->y = KinematicRect->position->y + KinematicRect->position->h + 0.01f;
				if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->ust = 1;
			}
			else {
				DynamicRect->position->x = KinematicRect->position->x - DynamicRect->position->w - 0.01f;
				if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sag = 1;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect->position->x + DynamicRect->position->w - KinematicRect->position->x) >= ((DynamicRect->position->y + DynamicRect->position->h - KinematicRect->position->y))) {
				DynamicRect->position->y = KinematicRect->position->y - DynamicRect->position->h - 0.01f;
				if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->alt = 1;
				if (gametype == SKR_SIDESCROLLER) {
					DynamicRect->isonground = 1;
					DynamicRect->position->y += 0.02f;
					if (KinematicRect->velocity.y > 0 && KinematicRect->velocity.y <= gravity) {
						DynamicRect->yk = KinematicRect->velocity.y;
					}
					DynamicRect->xk = KinematicRect->velocity.x;
				}
			}
			else {
				DynamicRect->position->x = KinematicRect->position->x - DynamicRect->position->w - 0.01f;
				if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sag = 1;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = KinematicRect->position->y + KinematicRect->position->h + 0.01f;
			if (DynamicRect->velocity.y < 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->ust = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = KinematicRect->position->x - DynamicRect->position->w - 0.01f;
			if (DynamicRect->velocity.x > 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sag = 1;
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = KinematicRect->position->y - DynamicRect->position->h - 0.01f;
			if (DynamicRect->velocity.y > 0) {
				DynamicRect->velocity.y = 0;
			}
			DynamicRect->alt = 1;
			if (gametype == SKR_SIDESCROLLER) {
				DynamicRect->isonground = 1;
				DynamicRect->position->y += 0.02f;
				if (KinematicRect->velocity.y > 0 && KinematicRect->velocity.y <= gravity) {
					DynamicRect->yk = KinematicRect->velocity.y;
				}
				DynamicRect->xk = KinematicRect->velocity.x;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = KinematicRect->position->x + KinematicRect->position->w + 0.01f;
			if (DynamicRect->velocity.x < 0) {
				DynamicRect->velocity.x = 0;
			}
			DynamicRect->sol = 1;
		}
	}
	return;
}

void CollideDynamicDynamic(SKR_DynamicRect* DynamicRect, SKR_DynamicRect* DynamicRect2) {
	if (DynamicRect != DynamicRect2 && SKR_IntersectRectRect(DynamicRect->position, DynamicRect2->position)) {
		*xmin = DynamicRect2->position->x;
		*ymin = DynamicRect2->position->y;
		*xmax = DynamicRect2->position->x + DynamicRect2->position->w;
		*ymax = DynamicRect2->position->y + DynamicRect2->position->h;
		if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = DynamicRect2->position->y - DynamicRect->position->h;
			if (DynamicRect2->alt == 1) {
				if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->isonground = 1;
				DynamicRect->alt = 1;
			}
			else if (DynamicRect->velocity.y > 0) {
				DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.y = DynamicRect->velocity.y;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = DynamicRect2->position->x + DynamicRect2->position->w;
			if (DynamicRect2->sol == 1) {
				if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sol = 1;
			}
			else if (DynamicRect->velocity.x < 0) {
				DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.x = DynamicRect->velocity.x;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = DynamicRect2->position->y + DynamicRect2->position->h;
			if (DynamicRect2->ust == 1) {
				if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->ust = 1;
			}
			else if (DynamicRect->velocity.y < 0) {
				DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.y = DynamicRect->velocity.y;
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = DynamicRect2->position->x - DynamicRect->position->w;
			if (DynamicRect2->sag == 1) {
				if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sag = 1;
			}
			else if (DynamicRect->velocity.x > 0) {
				DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.x = DynamicRect->velocity.x;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect2->position->x + DynamicRect2->position->w - DynamicRect->position->x) >= ((DynamicRect->position->y + DynamicRect->position->h - DynamicRect2->position->y))) {
				DynamicRect->position->y = DynamicRect2->position->y - DynamicRect->position->h;
				if (DynamicRect2->alt == 1) {
					if (DynamicRect->velocity.y > 0) {
						DynamicRect->velocity.y = 0;
					}
					DynamicRect->isonground = 1;
					DynamicRect->alt = 1;
				}
				else if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.y = DynamicRect->velocity.y;
				}
			}
			else {
				DynamicRect->position->x = DynamicRect2->position->x + DynamicRect2->position->w;
				if (DynamicRect2->sol == 1) {
					if (DynamicRect->velocity.x < 0) {
						DynamicRect->velocity.x = 0;
					}
					DynamicRect->sol = 1;
				}
				else if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.x = DynamicRect->velocity.x;
				}
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect2->position->x + DynamicRect2->position->w - DynamicRect->position->x) >= ((DynamicRect2->position->y + DynamicRect2->position->h - DynamicRect->position->y) + DynamicRect->velocity.y)) {
				DynamicRect->position->y = DynamicRect2->position->y + DynamicRect2->position->h;
				if (DynamicRect2->ust == 1) {
					if (DynamicRect->velocity.y < 0) {
						DynamicRect->velocity.y = 0;
					}
					DynamicRect->ust = 1;
				}
				else if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.y = DynamicRect->velocity.y;
				}
			}
			else {
				DynamicRect->position->x = DynamicRect2->position->x + DynamicRect2->position->w;
				if (DynamicRect2->sol == 1) {
					if (DynamicRect->velocity.x < 0) {
						DynamicRect->velocity.x = 0;
					}
					DynamicRect->sol = 1;
				}
				else if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.x = DynamicRect->velocity.x;
				}
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect->position->x + DynamicRect->position->w - DynamicRect2->position->x) >= ((DynamicRect2->position->y + DynamicRect2->position->h - DynamicRect->position->y) + DynamicRect->velocity.y)) {
				DynamicRect->position->y = DynamicRect2->position->y + DynamicRect2->position->h;
				if (DynamicRect2->ust == 1) {
					if (DynamicRect->velocity.y < 0) {
						DynamicRect->velocity.y = 0;
					}
					DynamicRect->ust = 1;
				}
				else if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.y = DynamicRect->velocity.y;
				}
			}
			else {
				DynamicRect->position->x = DynamicRect2->position->x - DynamicRect->position->w;
				if (DynamicRect2->sag == 1) {
					if (DynamicRect->velocity.x > 0) {
						DynamicRect->velocity.x = 0;
					}
					DynamicRect->sag = 1;
				}
				else if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.x = DynamicRect->velocity.x;
				}
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			if ((DynamicRect->position->x + DynamicRect->position->w - DynamicRect2->position->x) >= ((DynamicRect->position->y + DynamicRect->position->h - DynamicRect2->position->y))) {
				DynamicRect->position->y = DynamicRect2->position->y - DynamicRect->position->h;
				if (DynamicRect2->alt == 1) {
					if (DynamicRect->velocity.y > 0) {
						DynamicRect->velocity.y = 0;
					}
					DynamicRect->isonground = 1;
					DynamicRect->alt = 1;
				}
				else if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.y = DynamicRect->velocity.y;
				}
			}
			else {
				DynamicRect->position->x = DynamicRect2->position->x - DynamicRect->position->w;
				if (DynamicRect2->sag == 1) {
					if (DynamicRect->velocity.x > 0) {
						DynamicRect->velocity.x = 0;
					}
					DynamicRect->sag = 1;
				}
				else if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
					DynamicRect2->velocity.x = DynamicRect->velocity.x;
				}
			}
		}
		else if (!SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = DynamicRect2->position->y + DynamicRect2->position->h;
			if (DynamicRect2->ust == 1) {
				if (DynamicRect->velocity.y < 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->ust = 1;
			}
			else if (DynamicRect->velocity.y < 0) {
				DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.y = DynamicRect->velocity.y;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = DynamicRect2->position->x - DynamicRect->position->w;
			if (DynamicRect2->sag == 1) {
				if (DynamicRect->velocity.x > 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sag = 1;
			}
			else if (DynamicRect->velocity.x > 0) {
				DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.x = DynamicRect->velocity.x;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->y = DynamicRect2->position->y - DynamicRect->position->h;
			if (DynamicRect2->alt == 1) {
				if (DynamicRect->velocity.y > 0) {
					DynamicRect->velocity.y = 0;
				}
				DynamicRect->isonground = 1;
				DynamicRect->alt = 1;
			}
			else if (DynamicRect->velocity.y > 0) {
				DynamicRect->velocity.y = ((DynamicRect->velocity.y * DynamicRect->mass) + (DynamicRect2->velocity.y * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.y = DynamicRect->velocity.y;
			}
		}
		else if (SKR_IntersectRectLine(DynamicRect->position, xmin, ymin, xmax, ymin) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymin, xmax, ymax) && SKR_IntersectRectLine(DynamicRect->position, xmax, ymax, xmin, ymax) && !SKR_IntersectRectLine(DynamicRect->position, xmin, ymax, xmin, ymin)) {
			DynamicRect->position->x = DynamicRect2->position->x + DynamicRect2->position->w;
			if (DynamicRect2->sol == 1) {
				if (DynamicRect->velocity.x < 0) {
					DynamicRect->velocity.x = 0;
				}
				DynamicRect->sol = 1;
			}
			else if (DynamicRect->velocity.x < 0) {
				DynamicRect->velocity.x = ((DynamicRect->velocity.x * DynamicRect->mass) + (DynamicRect2->velocity.x * DynamicRect2->mass)) / (DynamicRect->mass + DynamicRect2->mass);
				DynamicRect2->velocity.x = DynamicRect->velocity.x;
			}
		}
	}
	return;
}

SKR_RectWorld* SKR_CreateRectWorld(float Gravity, float AirFrictionCoefficient, SKR_GAMETYPE GameType) {
	if (xmax == NULL) {
		xmax = malloc(sizeof(float));
	}
	if (xmin == NULL) {
		xmin = malloc(sizeof(float));
	}
	if (ymax == NULL) {
		ymax = malloc(sizeof(float));
	}
	if (ymin == NULL) {
		ymin = malloc(sizeof(float));
	}
	SKR_RectWorld* world = malloc(sizeof(SKR_RectWorld));
	world->gravity = Gravity;
	world->airfriction = AirFrictionCoefficient;
	world->StaticRectList = NULL;
	world->KinematicRectList = NULL;
	world->DynamicRectList = NULL;
	world->gametype = GameType;
	if (GameType == SKR_ISOMETRIC) {
		world->gravity = 0;
	}
	return world;
}

void SKR_SetGravity(SKR_RectWorld* World, float Gravity) {
	if (World != NULL) {
		World->gravity = Gravity;
	}
	return;
}

float SKR_GetGravity(SKR_RectWorld* World) {
	if (World != NULL) {
		return World->gravity;
	}
	return 0;
}

void SKR_SetAirFriction(SKR_RectWorld* World, float AirFrictionCoefficient) {
	if (World != NULL) {
		World->airfriction = AirFrictionCoefficient;
	}
	return;
}

float SKR_GetAirFriction(SKR_RectWorld* World) {
	if (World != NULL) {
		return World->airfriction;
	}
	return 0;
}

void SKR_DestroyRectWorld(SKR_RectWorld* World) {
	SKR_StaticRect* tmp = World->StaticRectList;
	while (World->StaticRectList != NULL) {
		tmp = World->StaticRectList;
		World->StaticRectList = World->StaticRectList->sonraki;
		free(tmp);
		tmp = NULL;
	}
	SKR_KinematicRect* tmp2 = World->KinematicRectList;
	while (World->KinematicRectList != NULL) {
		tmp2 = World->KinematicRectList;
		World->KinematicRectList = World->KinematicRectList->sonraki;
		free(tmp2);
		tmp2 = NULL;
	}
	SKR_DynamicRect* tmp3 = World->DynamicRectList;
	while (World->DynamicRectList != NULL) {
		tmp3 = World->DynamicRectList;
		World->DynamicRectList = World->DynamicRectList->sonraki;
		free(tmp3);
		tmp3 = NULL;
	}
	free(World);
	World = NULL;
	return;
}

SKR_StaticRect* SKR_CreateStaticRect(SKR_RectWorld* World, SKR_Rect* Position) {
	if (World == NULL) {
		return NULL;
	}
	SKR_StaticRect* yeni = malloc(sizeof(SKR_StaticRect));
	yeni->position = Position;
	yeni->sonraki = World->StaticRectList;
	World->StaticRectList = yeni;
	return yeni;
}

void SKR_DestroyStaticRect(SKR_RectWorld* World, SKR_StaticRect* StaticRect) {
	if (World->StaticRectList == StaticRect) {
		World->StaticRectList = World->StaticRectList->sonraki;
		free(StaticRect);
		StaticRect = NULL;
		return;
	}
	SKR_StaticRect* tmp = World->StaticRectList;
	SKR_StaticRect* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == StaticRect) {
			tmp2->sonraki = tmp->sonraki;
			free(StaticRect);
			StaticRect = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->sonraki;
	}
	return;
}

SKR_Rect* SKR_GetPositionStaticRect(SKR_StaticRect* StaticRect) {
	if (StaticRect != NULL) {
		return StaticRect->position;
	}
	else {
		return NULL;
	}
}

int SKR_GetStaticRectNumber(SKR_RectWorld* World) {
	if (World == NULL) {
		return 0;
	}
	int size = 0;
	SKR_StaticRect* tmp = World->StaticRectList;
	while (tmp != NULL) {
		size++;
		tmp = tmp->sonraki;
	}
	return size;
}

SKR_KinematicRect* SKR_CreateKinematicRect(SKR_RectWorld* World, SKR_Rect* Position) {
	if (World == NULL) {
		return NULL;
	}
	SKR_KinematicRect* yeni = malloc(sizeof(SKR_KinematicRect));
	yeni->position = Position;
	yeni->sonraki = World->KinematicRectList;
	yeni->velocity.x = 0;
	yeni->velocity.y = 0;
	yeni->boolean = -1;
	World->KinematicRectList = yeni;
	return yeni;
}

void SKR_DestroyKinematicRect(SKR_RectWorld* World, SKR_KinematicRect* KinematicRect) {
	if (World->KinematicRectList == KinematicRect) {
		World->KinematicRectList = World->KinematicRectList->sonraki;
		free(KinematicRect);
		KinematicRect = NULL;
		return;
	}
	SKR_KinematicRect* tmp = World->KinematicRectList;
	SKR_KinematicRect* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == KinematicRect) {
			tmp2->sonraki = tmp->sonraki;
			free(KinematicRect);
			KinematicRect = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->sonraki;
	}
	return;
}

SKR_Rect* SKR_GetPositionKinematicRect(SKR_KinematicRect* KinematicRect) {
	if (KinematicRect != NULL) {
		return KinematicRect->position;
	}
	else {
		return NULL;
	}
}

int SKR_GetKinematicRectNumber(SKR_RectWorld* World) {
	if (World == NULL) {
		return 0;
	}
	int size = 0;
	SKR_KinematicRect* tmp = World->KinematicRectList;
	while (tmp != NULL) {
		size++;
		tmp = tmp->sonraki;
	}
	return size;
}

void SKR_SetXVelocityKinematicRect(SKR_KinematicRect* KinematicRect, float Xspeed) {
	if (KinematicRect != NULL) {
		KinematicRect->velocity.x = Xspeed;
	}
	return;
}

void SKR_SetYVelocityKinematicRect(SKR_KinematicRect* KinematicRect, float Yspeed) {
	if (KinematicRect != NULL) {
		KinematicRect->velocity.y = Yspeed;
	}
	return;
}

float SKR_GetXVelocityKinematicRect(SKR_KinematicRect* KinematicRect) {
	if (KinematicRect != NULL) {
		return KinematicRect->velocity.x;
	}
	else {
		return 0;
	}
}

float SKR_GetYVelocityKinematicRect(SKR_KinematicRect* KinematicRect) {
	if (KinematicRect != NULL) {
		return KinematicRect->velocity.y;
	}
	else {
		return 0;
	}
}

void SKR_AnimateKinematicRect(SKR_KinematicRect* KinematicRect, float X1, float Y1, float X2, float Y2, float Velocity) {
	Velocity = fabsf(Velocity);
	if (KinematicRect == NULL) {
		return;
	}
	else if (KinematicRect->boolean == -2) {
		return;
	}
	else if (KinematicRect->boolean == -1) {
		KinematicRect->position->x = X1;
		KinematicRect->position->y = Y1;
		KinematicRect->boolean = 0;
	}
	else if (KinematicRect->boolean == 0) {
		KinematicRect->velocity.x = ((X2 - X1) / sqrtf((powf(X2 - X1, 2) + powf(Y2 - Y1, 2)))) * Velocity;
		KinematicRect->velocity.y = ((Y2 - Y1) / sqrtf((powf(X2 - X1, 2) + powf(Y2 - Y1, 2)))) * Velocity;
		if (X1 > X2) {
			if (KinematicRect->position->x <= X2) {
				KinematicRect->position->x = X2;
				KinematicRect->position->y = Y2;
				KinematicRect->boolean = 1;
			}
		}
		else if (X1 < X2) {
			if (KinematicRect->position->x >= X2) {
				KinematicRect->position->x = X2;
				KinematicRect->position->y = Y2;
				KinematicRect->boolean = 1;
			}
		}
		else if (Y1 > Y2) {
			if (KinematicRect->position->x <= Y2) {
				KinematicRect->position->x = X2;
				KinematicRect->position->y = Y2;
				KinematicRect->boolean = 1;
			}
		}
		else if (Y1 < Y2) {
			if (KinematicRect->position->x >= Y2) {
				KinematicRect->position->x = X2;
				KinematicRect->position->y = Y2;
				KinematicRect->boolean = 1;
			}
		}
		else {
			KinematicRect->velocity.x = 0;
			KinematicRect->velocity.y = 0;
			KinematicRect->boolean = -2;
		}
	}
	else if (KinematicRect->boolean == 1) {
		KinematicRect->velocity.x = ((X1 - X2) / sqrtf((powf(X2 - X1, 2) + powf(Y2 - Y1, 2)))) * Velocity;
		KinematicRect->velocity.y = ((Y1 - Y2) / sqrtf((powf(X2 - X1, 2) + powf(Y2 - Y1, 2)))) * Velocity;
		if (X1 > X2) {
			if (KinematicRect->position->x >= X1) {
				KinematicRect->position->x = X1;
				KinematicRect->position->y = Y1;
				KinematicRect->boolean = 0;
			}
		}
		else if (X1 < X2) {
			if (KinematicRect->position->x <= X1) {
				KinematicRect->position->x = X1;
				KinematicRect->position->y = Y1;
				KinematicRect->boolean = 0;
			}
		}
		else if (Y1 > Y2) {
			if (KinematicRect->position->x >= Y1) {
				KinematicRect->position->x = X1;
				KinematicRect->position->y = Y1;
				KinematicRect->boolean = 0;
			}
		}
		else if (Y1 < Y2) {
			if (KinematicRect->position->x <= Y1) {
				KinematicRect->position->x = X1;
				KinematicRect->position->y = Y1;
				KinematicRect->boolean = 0;
			}
		}
		else {
			KinematicRect->velocity.x = 0;
			KinematicRect->velocity.y = 0;
			KinematicRect->boolean = -2;
		}
	}
	return;
}

void SKR_StopAnimatingKinematicRect(SKR_KinematicRect* KinematicRect) {
	if (KinematicRect == NULL) {
		return;
	}
	KinematicRect->boolean = -1;
	KinematicRect->velocity.x = 0;
	KinematicRect->velocity.y = 0;
	return;
}

SKR_DynamicRect* SKR_CreateDynamicRect(SKR_RectWorld* World, SKR_Rect* Position, float Mass, float FrictionCoefficient, float GravityMultiplier) {
	if (World == NULL) {
		return NULL;
	}
	SKR_DynamicRect* yeni = malloc(sizeof(SKR_DynamicRect));
	yeni->position = Position;
	yeni->sonraki = World->DynamicRectList;
	yeni->mass = Mass;
	yeni->friction = FrictionCoefficient;
	yeni->velocity.x = 0;
	yeni->velocity.y = 0;
	yeni->force.x = 0;
	yeni->force.y = 0;
	yeni->gravitymultiplier = GravityMultiplier;
	yeni->xk = 0;
	yeni->yk = 0;
	yeni->alt = 0;
	yeni->sol = 0;
	yeni->sag = 0;
	yeni->ust = 0;
	World->DynamicRectList = yeni;
	return yeni;
}

void SKR_DestroyDynamicRect(SKR_RectWorld* World, SKR_DynamicRect* DynamicRect) {
	if (World->DynamicRectList == DynamicRect) {
		World->DynamicRectList = World->DynamicRectList->sonraki;
		free(DynamicRect);
		DynamicRect = NULL;
		return;
	}
	SKR_DynamicRect* tmp = World->DynamicRectList;
	SKR_DynamicRect* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == DynamicRect) {
			tmp2->sonraki = tmp->sonraki;
			free(DynamicRect);
			DynamicRect = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->sonraki;
	}
	return;
}

SKR_Rect* SKR_GetPositionDynamicRect(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->position;
	}
	else {
		return NULL;
	}
}

int SKR_GetDynamicRectNumber(SKR_RectWorld* World) {
	if (World == NULL) {
		return 0;
	}
	int size = 0;
	SKR_DynamicRect* tmp = World->DynamicRectList;
	while (tmp != NULL) {
		size++;
		tmp = tmp->sonraki;
	}
	return size;
}

void SKR_SetXVelocityDynamicRect(SKR_DynamicRect* DynamicRect, float Xspeed) {
	if (DynamicRect != NULL) {
		DynamicRect->velocity.x = Xspeed;
	}
	return;
}

void SKR_SetYVelocityDynamicRect(SKR_DynamicRect* DynamicRect, float Yspeed) {
	if (DynamicRect != NULL) {
		DynamicRect->velocity.y = Yspeed;
	}
	return;
}

float SKR_GetXVelocityDynamicRect(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->velocity.x;
	}
	else {
		return 0;
	}
}

float SKR_GetYVelocityDynamicRect(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->velocity.y;
	}
	else {
		return 0;
	}
}

float SKR_GetMassDynamicRect(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->mass;
	}
	else {
		return 0;
	}
}

void SKR_SetMassDynamicRect(SKR_DynamicRect* DynamicRect, float Mass) {
	if (DynamicRect != NULL) {
		DynamicRect->mass = Mass;
	}
	return;
}

void SKR_ApplyForceX(SKR_DynamicRect* DynamicRect, float Force) {
	if (DynamicRect != NULL) {
		DynamicRect->force.x = DynamicRect->force.x + Force;
	}
	return;
}

void SKR_ApplyForceY(SKR_DynamicRect* DynamicRect, float Force) {
	if (DynamicRect != NULL) {
		DynamicRect->force.y = DynamicRect->force.y + Force;
	}
	return;
}

void SKR_SetForceX(SKR_DynamicRect* DynamicRect, float Force) {
	if (DynamicRect != NULL) {
		DynamicRect->force.x = Force;
	}
	return;
}

void SKR_SetForceY(SKR_DynamicRect* DynamicRect, float Force) {
	if (DynamicRect != NULL) {
		DynamicRect->force.y = Force;
	}
	return;
}

float SKR_GetForceX(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->force.x;
	}
	return 0;
}

float SKR_GetForceY(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->force.y;
	}
	return 0;
}

void SKR_SimulateWorld(SKR_RectWorld* World, float Milliseconds) {
	tmpD = World->DynamicRectList;
	tmpD2 = World->DynamicRectList;
	tmpS = World->StaticRectList;
	tmpK = World->KinematicRectList;
	while (tmpK != NULL) {
		tmpK->position->x = tmpK->position->x + (tmpK->velocity.x * (Milliseconds / 10));
		tmpK->position->y = tmpK->position->y + (tmpK->velocity.y * (Milliseconds / 10));
		tmpK = tmpK->sonraki;
	}
	while (tmpD != NULL) {
		xmax2 = tmpD->velocity.x;
		tmpD->velocity.x = tmpD->velocity.x + (tmpD->force.x / tmpD->mass);
		if (tmpD->velocity.x > 0) {
			tmpD->velocity.x = tmpD->velocity.x - fabsf(xmax2 * World->airfriction * (Milliseconds / 1000));
			if (tmpD->alt == 1 || tmpD->ust == 1) {
				tmpD->velocity.x = tmpD->velocity.x - fabsf(xmax2 * tmpD->friction * (Milliseconds / 1000));
			}
			if (tmpD->velocity.x < 0.01f) {
				tmpD->velocity.x = 0;
			}
		}
		else if (tmpD->velocity.x < 0) {
			tmpD->velocity.x = tmpD->velocity.x + fabsf(xmax2 * World->airfriction * (Milliseconds / 1000));
			if (tmpD->alt == 1 || tmpD->ust == 1) {
				tmpD->velocity.x = tmpD->velocity.x + fabsf(xmax2 * tmpD->friction * (Milliseconds / 1000));
			}
			if (tmpD->velocity.x > -0.01f) {
				tmpD->velocity.x = 0;
			}
		}
		ymax2 = tmpD->velocity.y;
		tmpD->velocity.y = tmpD->velocity.y + (tmpD->force.y / tmpD->mass) + (World->gravity * (tmpD->gravitymultiplier) * (Milliseconds / 1000));
		if (tmpD->velocity.y > 0) {
			tmpD->velocity.y = tmpD->velocity.y - fabsf(ymax2 * World->airfriction * (Milliseconds / 1000));
			if (tmpD->sol == 1 || tmpD->sag == 1) {
				tmpD->velocity.y = tmpD->velocity.y - fabsf(ymax2 * tmpD->friction * (Milliseconds / 1000));
			}
			if (tmpD->velocity.y < 0.01f) {
				tmpD->velocity.y = 0;
			}
		}
		else if (tmpD->velocity.y < 0) {
			tmpD->velocity.y = tmpD->velocity.y + fabsf(ymax2 * World->airfriction * (Milliseconds / 1000));
			if (tmpD->sol == 1 || tmpD->sag == 1) {
				tmpD->velocity.y = tmpD->velocity.y + fabsf(ymax2 * tmpD->friction * (Milliseconds / 1000));
			}
			if (tmpD->velocity.y > -0.01f) {
				tmpD->velocity.y = 0;
			}
		}
		tmpD->force.x = 0;
		tmpD->force.y = 0;
		xmax2 = ((tmpD->velocity.x + tmpD->xk) * (Milliseconds / 10));
		ymax2 = ((tmpD->velocity.y + tmpD->yk) * (Milliseconds / 10));
		xmin2 = max(fabsf(xmax2), fabsf(ymax2));
		loop = (int)xmin2;
		if (loop < 1) {
			loop = 1;
		}
		tmpD->isonground = 0;
		tmpD->xk = 0;
		tmpD->yk = 0;
		tmpD->sol = 0;
		tmpD->sag = 0;
		tmpD->alt = 0;
		tmpD->ust = 0;
		if (xmin2 == 0) {
			tmpS = World->StaticRectList;
			while (tmpS != NULL) {
				CollideDynamicStatic(tmpD, tmpS);
				tmpS = tmpS->sonraki;
			}
			tmpK = World->KinematicRectList;
			while (tmpK != NULL) {
				CollideDynamicKinematic(tmpD, tmpK, (World->gravity * (tmpD->gravitymultiplier)), World->gametype);
				tmpK = tmpK->sonraki;
			}
			tmpD2 = World->DynamicRectList;
			while (tmpD2 != NULL) {
				CollideDynamicDynamic(tmpD, tmpD2);
				tmpD2 = tmpD2->sonraki;
			}
		}
		else {
			for (i = 0; i < loop; i++) {
				tmpD->position->x = tmpD->position->x + (xmax2 / loop);
				tmpD->position->y = tmpD->position->y + (ymax2 / loop);
				tmpS = World->StaticRectList;
				while (tmpS != NULL) {
					CollideDynamicStatic(tmpD, tmpS);
					tmpS = tmpS->sonraki;
				}
				tmpK = World->KinematicRectList;
				while (tmpK != NULL) {
					CollideDynamicKinematic(tmpD, tmpK, (World->gravity * (tmpD->gravitymultiplier)), World->gametype);
					tmpK = tmpK->sonraki;
				}
				tmpD2 = World->DynamicRectList;
				while (tmpD2 != NULL) {
					CollideDynamicDynamic(tmpD, tmpD2);
					tmpD2 = tmpD2->sonraki;
				}
			}
		}
		if (World->gametype == SKR_ISOMETRIC) {
			tmpD->isonground = 0;
		}
		tmpD = tmpD->sonraki;
	}
	return;
}

int SKR_IsOnground(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->isonground;
	}
	else {
		return 0;
	}
}

float SKR_GetFriction(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->friction;
	}
	return 0;
}

void SKR_SetFriction(SKR_DynamicRect* DynamicRect, float FrictionCoefficient) {
	if (DynamicRect != NULL) {
		DynamicRect->friction = FrictionCoefficient;
	}
	return;
}

float SKR_GetGravityMultiplier(SKR_DynamicRect* DynamicRect) {
	if (DynamicRect != NULL) {
		return DynamicRect->gravitymultiplier;
	}
	return 0;
}

void SKR_SetGravityMultiplier(SKR_DynamicRect* DynamicRect, float GravityMultiplier) {
	if (DynamicRect != NULL) {
		DynamicRect->gravitymultiplier = GravityMultiplier;
	}
	return;
}