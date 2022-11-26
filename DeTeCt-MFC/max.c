/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    MAX: Impact detection by individual frames analysis functions				*/
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdio.h>
#include <math.h>

#include "max.h"
#include <windows.h>
#include <strsafe.h>

LIST *init_list(LIST *list, int maxsize)
{
	list->size = 0;
	list->maxsize = maxsize;
	list->head = list->tail = NULL;
	
	return list;
}

POINT_FRAME *create_point(long frame, double val, int x, int y)
{
	POINT_FRAME *p = NULL;
	
	p = (POINT_FRAME *) calloc(1, sizeof (POINT_FRAME));
	
	if (p)
	{
		p->frame = frame;
		p->val = val;
		p->x = x;
		p->y = y;
	}
	
	return p;
}

ITEM *create_item(POINT_FRAME *p)
{
	ITEM *i;
	
	i = (ITEM *) calloc(1, sizeof (ITEM));
	
	if (i)
	{
		if (p)
			i->point = p;
		else
			i->point = NULL;
		
		i->next = i->prev = NULL;
	}
	
	return i;
}

void delete_head_item(LIST *list)
{
	ITEM *item;
	
	if (!list) return;
	
	if (!(item = list->head)) return;

	if (list->head == list->tail)
	{
		list->tail = NULL;
	}
	
	list->head = item->next;
	
	if (item->next)
	{
		item->next->prev = NULL;
	}
	
	free(item->point);
	free(item); 
	
	list->size--;
}

void add_tail_item(LIST *list, ITEM *item)
{
	int c;
	
	if (!(list->head))
	{
		list->head = list->tail = item;	
		item->prev = item->next = NULL;
		list->size = 1;
	}
	else
	{
		item->prev = list->tail;
		item->next = NULL;
		list->tail->next = item;
		list->tail = item;
		list->size++;
		if ((c = (list->size - list->maxsize)) > 0)
		{
			for (int i = 0; i < c; i++)
			{
				delete_head_item(list);
			}
		}
	}
}

void delete_list(LIST *list)
{
	while (list->size > 0) {
		delete_head_item(list);
	}
}

/*
void print_list(LIST *list)
{
	ITEM *item;
	
	item = list->head;
	
	printf("----------------------------------------------------------------------\n");
	while (item)// && item != list->tail)
	{
		printf("Size: %d\tframe:%3ld\tval= %f\t x=%3d y=%3d\n", list->size, item->point->frame, item->point->val, item->point->x, item->point->y);
		item = item->next;
	}
	printf("Head: %p\tframe:%3ld\tval= %f\t x=%3d y=%3d\n", list->head, list->head->point->frame, list->head->point->val, list->head->point->x, list->head->point->y);
	printf("Tail: %p\tframe:%3ld\tval= %f\t x=%3d y=%3d\n", list->tail, list->tail->point->frame, list->tail->point->val, list->tail->point->x, list->tail->point->y);
	printf("----------------------------------------------------------------------\n");
}

void print_array(ITEM **array, int size)
{
	int c;
	
	printf("----------------------------------------------------------------------\n");
	for (c = 0; c < size; c++) {
		printf("Size: %d\tframe:%3ld\tval= %f\t x=%3d y=%3d\n", size, array[c]->point->frame, array[c]->point->val, array[c]->point->x, array[c]->point->y);
	}
	printf("----------------------------------------------------------------------\n");
}
*/

int itemcmp(const void *a, const void *b)
{
	if ((*((ITEM **) a))->point->val < (*((ITEM **) b))->point->val) return 1;
	else if ((*((ITEM **) a))->point->val > (*((ITEM **) b))->point->val) return -1;
	else return 0; 
}

/* int detect_impact(DTCIMPACT *dtc, LIST *list, int fps, double radius, double incrLum, int incrFrame)
{
	int c, n, minC, maxC;
	int x0, y0;
	int minFrame, maxFrame;
	int ivalFrame, lastivalFrame;
	long hini, mini, sini;
	long hfin, mfin, sfin;
	double meanValue;
	double d;
	ITEM **ord, **tmp;
	ITEM *tmpSrc;
	int nb_impact;

	nb_impact=0;
	if (list->size<=0) return 0;
	if (!(ord = (ITEM **) calloc(list->size, sizeof (ITEM *)))) {
			//fprintf(stderr, "ERROR in detect_impact: get_max_list: cannot reserve memory\n");
		OutputDebugString(L"ERROR in detect_impact: get_max_list: cannot reserve memory\n");
			exit(EXIT_FAILURE);
	}
	for (tmpSrc = list->head, tmp = ord, c = 0; tmpSrc && c < list->size; tmpSrc = tmpSrc->next, tmp++, c++)
			*tmp = tmpSrc;
	qsort(ord, list->size, sizeof (ITEM *), itemcmp);
	meanValue = get_item_list_mean_value(list);
	TCHAR buffer[1000];
	// /*StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("%s %f\n"), TEXT("Max-Mean value:"), ord[0]->point->val - meanValue);
	// OutputDebugString(buffer);
	// StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("%s %f\n"), TEXT("Mean value times increase:"), meanValue*(1 + incrLum));
	// OutputDebugString(buffer);
	// StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("%s %f\n"), TEXT("Max brightness value:"), ord[0]->point->val);
	// OutputDebugString(buffer);
	// StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("%s %f\n"), TEXT("x value:"), (ord[0]->point->val / meanValue) - 1);
	// OutputDebugString(buffer);**
	if (ord[0]->point->val <= meanValue*(1 + incrLum)) {
		//double x = (meanValue / ord[0]->point->val) - 1;
		//OutputDebugString(L"Not an impact candidate\n");
		return nb_impact;
	} else {
		//OutputDebugString(L"Impact candidate\n");
	}
	x0 = ord[0]->point->x;
	y0 = ord[0]->point->y;
	dtc->MaxFrame  = ord[0]->point->frame;

	for (n = 0, d = 0.0; n < list->size && d <= radius; n++)
		d = sqrt(pow(ord[n]->point->x - x0, 2) + pow(ord[n]->point->y - y0, 2));
	n--;
	
	for (maxC = minC = c = 0, minFrame = maxFrame = ord[0]->point->frame; c < n; c++) {
		if (minFrame > ord[c]->point->frame) {
			minFrame = ord[c]->point->frame;
			minC = c;
		}
		if (maxFrame < ord[c]->point->frame) {
			maxFrame = ord[c]->point->frame;
			maxC = c;
		}
	}
	ivalFrame = ord[maxC]->point->frame - ord[minC]->point->frame;

	lastivalFrame = dtc->nMaxFrame - dtc->nMinFrame;
	dtc->MaxFrame = ord[0]->point->frame;
	dtc->nMinFrame = ord[minC]->point->frame;
	dtc->nMaxFrame = ord[maxC]->point->frame;

	if (ivalFrame == lastivalFrame || !lastivalFrame) {
		if (n >= incrFrame) {
			sini = ord[minC]->point->frame / fps;
			hini = sini / 3600;
			sini %= 3600;
			mini = sini / 60;
			sini %= 60;
			sfin = ord[maxC]->point->frame / fps;
			hfin = sfin / 3600;
			sfin %= 3600;
			mfin = sfin / 60;
			sfin %= 60;
			// /*printf("Brightness increase: frame %ld (%ldh %02ldmin %02lds) "
			//       "to %ld (%ldh %02ldmin %02lds). ", 
			//       ord[minC]->point->frame, hini, mini, sini,
			//       ord[maxC]->point->frame, hfin, mfin, sfin);
			//printf("Max lum %d at frame %ld.\n", (int) ord[0]->point->val, ord[0]->point->frame);**
			StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("Brightness increase: frame %ld (%ldh % 02ldmin % 02lds) to %ld (%ldh %02ldmin %02lds).\n"),
				ord[minC]->point->frame, hini, mini, sini, ord[maxC]->point->frame, hfin, mfin, sfin);
			OutputDebugString(buffer);
			StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("Max lum %d at frame %ld\n."), (int)ord[0]->point->val, ord[0]->point->frame);
			OutputDebugString(buffer);
			fflush(stdout);
			nb_impact++;
			delete_list(list);
		}
	}
	free(ord);

	return nb_impact;
}
*/

double get_item_array_mean_value(ITEM **l, int n)
{
	int i;
	double val;
	
	val=0;
	for (i = 0; i < n; i++, l++)
	{
		val += (*l)->point->val;
	}
	return val/n;
}

double get_item_list_mean_value(LIST *l)
{
	int c;
	double val;
	ITEM *item;
	
	for (val = 0.0, c = 0, item = l->head; item; item = item->next, c++)
	{
		val += item->point->val;
	}

	return c > 0 ? val/c : 0;
}

void init_dtc_struct(DTCIMPACT *dtc)
{
	dtc->MaxFrame = dtc->nMaxFrame = dtc->nMinFrame = 0;
}

void print_list_item(LIST *l, int max)
{
	ITEM *item;
	int c;
	
	item = l->head;
	c = 0;
	
	printf("---\n");	
	while (item)
	{
		printf("%4ld(%9.2f) ", item->point->frame, item->point->val);
		item = item->next;
		if ((c % max) + 1 == max)
		{
			c = 0;
			printf("\n");
		}
		else
			c++;
			
	}
	printf("\n");
}

void print_item_array(ITEM **ord, size_t n, size_t max)
{
	 for (size_t c = 0; c < n && c < max; c++)
		printf("Frame %5ld: val=%8.2f (%4d,%4d)\n",
		       ord[c]->point->frame, ord[c]->point->val,
		       ord[c]->point->x, ord[c]->point->y);		       
}
