#ifndef __MAX_H__
#define __MAX_H__
#include "common.h"

#include <stdlib.h>

struct _point
{
	long	frame;			// index of frame
	double	val;			// value of differential photometry
	int		x;				// x in image
	int		y;				// y in image
};
typedef struct _point POINT_FRAME;

struct item					// point, previous and next items
{
	struct _point	*point;
	struct item		*next;
	struct item		*prev;
};
typedef struct item   ITEM;

struct list					// of items
{
	int			size;		//number of items
	int			maxsize;
	struct item *head;		//first item
	struct item *tail;		//last item
};
typedef struct list   LIST;

struct dtcImpact			//impact candidate frames
{
	long MaxFrame;
	long nMinFrame;
	long nMaxFrame;
};
typedef struct dtcImpact DTCIMPACT;


/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/
	
LIST 		*init_list(LIST *list, int maxsize);
POINT_FRAME *create_point(long frame, double val, int x, int y);
ITEM 		*create_item(POINT_FRAME *p);
void 		add_tail_item(LIST *list, ITEM *item);
void 		delete_list(LIST *list);
double 		get_item_point_val_list_mean_value(LIST *l);
void 		init_dtc_struct(DTCIMPACT *dtc);
int			item_point_val_cmp(const void* a, const void* b);					 // 1 if vala < valb, 0 if =, -1 if >

//int 		detect_impact(DTCIMPACT *dtc, LIST *list, int fps, double radius, double incrLum, int incrFrame);

#endif /* __MAX_H__ */
