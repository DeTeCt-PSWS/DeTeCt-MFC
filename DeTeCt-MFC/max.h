#ifndef __MAX_H__
#define __MAX_H__
#include "common.h"

#include <stdlib.h>

struct _point
{
	long frame;
	double val;
	int x;
	int y;
};

struct item
{
	struct _point *point;
	struct item *next;
	struct item *prev;
};

struct list
{
	int size;
	int maxsize;
	struct item *head;
	struct item *tail;
};

struct dtcImpact
{
	long MaxFrame;
	long nMinFrame;
	long nMaxFrame;
};

typedef struct _point POINT_FRAME;
typedef struct list   LIST;
typedef struct item   ITEM;
typedef struct dtcImpact DTCIMPACT;

/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/
	
LIST 		*init_list(LIST *list, int maxsize);
POINT_FRAME *create_point(long frame, double val, int x, int y);
ITEM 		*create_item(POINT_FRAME *p);
void 		delete_head_item(LIST *list);
void 		add_tail_item(LIST *list, ITEM *item);
void 		delete_list(LIST *list);
//int 		detect_impact(DTCIMPACT *dtc, LIST *list, int fps, double radius, double incrLum, int incrFrame);
double 		get_item_array_mean_value(ITEM **l, int n);
double 		get_item_list_mean_value(LIST *l);
void 		init_dtc_struct(DTCIMPACT *dtc);
void 		print_list_item(LIST *l, int max);
void 		print_item_array(ITEM **ord, size_t n, size_t max);

#endif /* __MAX_H__ */
