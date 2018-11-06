# Windows make file for dtc
# mingw32-make

OBJS = dtc.o cmdline.o img.o max.o wrapper.o serfmt.o datation.o filefmt.o fitsfmt.o common.o
#OBJS_TEST= dtc_test_video.o cmdline.o

LIBS = libopencv_core231.dll libopencv_highgui231.dll libopencv_imgproc231.dll libopencv_video231.dll opencv_ffmpeg.dll libfits.dll
INC = ../OpenCV-2.3.1/build/include -Ilibfits

#LIBS = libopencv_core246.dll libopencv_highgui246.dll libopencv_imgproc246.dll libopencv_video246.dll opencv_ffmpeg.dll libfits.dll
#avcodec-55.dll avdevice-55.dll avfilter-3.dll avformat-55.dll avutil-52.dll postproc-52.dll swresample-0.dll swscale-2.dll
#LIBS = libopencv_core246.dll libopencv_highgui246.dll libopencv_imgproc246.dll libopencv_video246.dll opencv_ffmpeg.dll libfits.dll libopencv_calib3d246.dll libopencv_contrib246.dll libopencv_features2d246.dll libopencv_flann246.dll  libopencv_gpu246.dll libopencv_legacy246.dll libopencv_ml246.dll libopencv_nonfree246.dll libopencv_objdetect246.dll libopencv_photo246.dll libopencv_stitching246.dll libopencv_videostab246.dll 
#INC = ../OpenCV-2.4.6/build/include -Ilibfits

CC = gcc.exe -O3 -static-libgcc -static-libstdc++ -Waddress -Warray-bounds -Wchar-subscripts -Wenum-compare -Wimplicit-int -Wimplicit-function-declaration -Wcomment -Wformat -Wmain -Wmaybe-uninitialized -Wmissing-braces -Wnonnull -Wparentheses -Wpointer-sign -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wextra
#-Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wmissing-parameter-type -Wold-style-declaration -Woverride-init -Wsign-compare -Wtype-limits -Wuninitialized -Wunused-parameter -Wunused-but-set-parameter 
#-Wall
#-Wpedantic -Wextra

.PHONY: clean

all: dtc.exe
#dtc_test_video.exe

dtc.exe: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

#dtc_test_video.exe: dtc_test_video.o cmdline.o
#	$(CC) $(OBJS_TEST) $(LIBS) -o $@
 
clean:
	del /F $(OBJS)
#	del /F $(OBJS_TEST)
	
dtc.o: dtc.c dtc.h img.h
	$(CC) -c $< -I$(INC) -o $@

#dtc_test_video.o: dtc_test_video.c cmdline.h
	$(CC) -c $< -I$(INC) -o $@
	
common.o: common.c common.h
	$(CC) -c $< -I$(INC) -o $@

cmdline.o: cmdline.c cmdline.h dtc.h common.h
	$(CC) -c $< -I$(INC) -o $@

max.o: max.c max.h common.h
	$(CC) -c $< -I$(INC) -o $@

wrapper.o: wrapper.c wrapper.h serfmt.h common.h
	$(CC) -c $< -I$(INC) -o $@

serfmt.o: serfmt.c serfmt.h datation.h wrapper.h dtc.h common.h
	$(CC) -c $< -I$(INC) -o $@

datation.o: datation.c datation.h wrapper.h dtc.h common.h
	$(CC) -c $< -I$(INC) -o $@
	
fitsfmt.o: fitsfmt.c fitsfmt.h wrapper.h dtc.h datation.h common.h
	$(CC) -c $< -I$(INC) -o $@

filefmt.o: filefmt.c filefmt.h wrapper.h dtc.h datation.h common.h
	$(CC) -c $< -I$(INC) -o $@

fitsfile.o: fitsfile.c fitsfile.h
	$(CC) -Wunused-function -c $< -I$(INC) -o $@

hget.o: hget.c fitshead.h
	$(CC) -Wunused-function -c $< -I$(INC) -o $@

hput.o: hput.c fitshead.h
	$(CC) -Wunused-function -c $< -I$(INC) -o $@

iget.o: iget.c fitshead.h
	$(CC) -Wunused-function -c $< -I$(INC) -o $@

img.o: img.c img.h cmdline.h wrapper.h common.h
	$(CC) -c $< -I$(INC) -o $@

wrapper.h: serfmt.h fitsfmt.h

img.h: wrapper.h

datation.h: wrapper.h

fitsfile.h: fitshead.h