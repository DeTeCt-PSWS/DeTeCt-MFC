#pragma once

#ifndef __FILEFIND_H__
#define __FILEFIND_H__

// Expose procedure declaration to both C and C++
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void GetCreatedModifiedTimes(const char *filename, double *pcreated_time, double *pmodified_time);

#endif /* __FILEFIND_H__ */