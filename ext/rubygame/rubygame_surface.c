/*
 *  Rubygame binding to SDL Surface class.
 *--
 *  Rubygame -- Ruby code and bindings to SDL to facilitate game creation
 *  Copyright (C) 2004-2007  John Croisant
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *++
 */

#include "rubygame_shared.h"
#include "rubygame_surface.h"

void Rubygame_Init_Surface();

VALUE rbgm_surface_new(int, VALUE*, VALUE);

VALUE rbgm_surface_get_w(VALUE);
VALUE rbgm_surface_get_h(VALUE);
VALUE rbgm_surface_get_size(VALUE);

VALUE rbgm_surface_get_depth(VALUE);
VALUE rbgm_surface_get_flags(VALUE);
VALUE rbgm_surface_get_masks(VALUE);

VALUE rbgm_surface_get_alpha(VALUE);
VALUE rbgm_surface_set_alpha(int, VALUE*, VALUE);

VALUE rbgm_surface_get_colorkey(VALUE);
VALUE rbgm_surface_set_colorkey(int, VALUE*, VALUE);

VALUE rbgm_surface_blit(int, VALUE*, VALUE);

VALUE rbgm_surface_fill(int, VALUE*, VALUE);

VALUE rbgm_surface_getat(VALUE, VALUE, VALUE);

VALUE rbgm_surface_pixels(VALUE);

VALUE rbgm_surface_get_cliprect(VALUE);
VALUE rbgm_surface_set_cliprect(VALUE, VALUE);

VALUE rbgm_surface_convert(int, VALUE*, VALUE);
VALUE rbgm_surface_displayformat(VALUE);
VALUE rbgm_surface_displayformatalpha(VALUE);

VALUE rbgm_image_savebmp(VALUE, VALUE);

VALUE rbgm_transform_flip(VALUE, VALUE, VALUE);

/* 
 *  call-seq:
 *     new(size, depth=0, flags=0)  ->  Surface
 *
 *  Create and initialize a new Surface object. 
 *
 *  A Surface is a grid of image data which you blit (i.e. copy) onto other
 *  Surfaces. Since the Rubygame display is also a Surface (see the Screen 
 *  class), Surfaces can be blit to the screen; this is the most common way
 *  to display images on the screen.
 *
 *  Currently, you must create a display window (see Screen#set_mode) before
 *  creating a new Surface.
 *
 *  This function takes these arguments:
 *  size::  requested surface size; an array of the form [width, height].
 *  depth:: requested color depth (in bits per pixel). If depth is 0 (default),
 *          use the color depth of the current Screen mode.
 *--
 *          automatically choose a color depth: either the depth of the Screen
 *          mode (if one has been set), or the greatest color depth available
 *          on the system.
 *++
 *  flags:: an Array or Bitwise-OR'd list of zero or more of the following 
 *          flags (located in the Rubygame module, e.g. Rubygame::SWSURFACE).
 *          This argument may be omitted, in which case the Surface 
 *          will be a normal software surface (this is not necessarily a bad
 *          thing).
 *          SWSURFACE::   (default) request a software surface.
 *          HWSURFACE::   request a hardware-accelerated surface (using a 
 *                        graphics card), if available. Creates a software
 *                        surface if hardware surfaces are not available.
 *          SRCCOLORKEY:: request a colorkeyed surface. #set_colorkey will
 *                        also enable colorkey as needed. For a description
 *                        of colorkeys, see #set_colorkey.
 *          SRCALPHA::    request an alpha channel. #set_alpha will
 *                        also enable alpha. as needed. For a description
 *                        of alpha, see #alpha.
 */
VALUE rbgm_surface_new(int argc, VALUE *argv, VALUE class)
{
	VALUE self;
	SDL_Surface *self_surf;
	SDL_PixelFormat* pixformat;
	Uint32 flags, Rmask, Gmask, Bmask, Amask;
	int w, h, depth, counter;
	

	if( SDL_GetVideoSurface() )
	{
		/* Pixel format is retrieved from the video surface. */
		pixformat = (SDL_GetVideoSurface())->format;
	}
	else
	{
		rb_raise(eSDLError,"Cannot create Surface before the Screen mode is set!");
		/* The following code causes an inexplicable segfault? :(  -j */
		/* pixformat = SDL_GetVideoInfo()->vfmt; */ 
	}

	Rmask = pixformat->Rmask;
	Gmask = pixformat->Gmask;
	Bmask = pixformat->Bmask;
	Amask = pixformat->Amask;

	if( (argc > 1) && (argv[1] != Qnil) && (argv[1] != 0) )
	{
		/* TODO: We might want to check that the requested depth makes sense. */
		depth = NUM2INT(argv[1]);
	}
	else
	{
		depth = pixformat->BitsPerPixel;
	}
		

	/* Get width and height for new surface from argv[0] */
	Check_Type(argv[0],T_ARRAY);

	if(RARRAY(argv[0])->len >= 2)
	{
		w = NUM2INT(rb_ary_entry(argv[0],0));
		h = NUM2INT(rb_ary_entry(argv[0],1));
	}
	else
		rb_raise(rb_eArgError,"wrong dimensions for Surface size (%d for 2)",\
			RARRAY(argv[0])->len);
	

	if(argc > 2 && argv[2] != Qnil)
	{
		switch( TYPE(argv[2]) ){
			case T_ARRAY:;
				for(counter=0;  counter < RARRAY(argv[2])->len; counter += 1)
		    {
		      flags |= NUM2UINT(  rb_ary_entry( argv[2],counter )  );
		    }
				break;
			case T_FIXNUM:;
				flags = NUM2UINT( argv[2] );
				break;
			default:;
				rb_raise(rb_eArgError,"Wrong type for argument `flags' (wanted Fixnum or Array).");
		}
	}

	/* Finally, we can create the new Surface! Or try, anyway... */
	self_surf = SDL_CreateRGBSurface(flags,w,h,depth,Rmask,Gmask,Bmask,Amask);

	if( self_surf == NULL )
		rb_raise(eSDLError,"Could not create new surface: %s",SDL_GetError());


	/* Wrap the new surface in a crunchy candy VALUE shell. */
	self = Data_Wrap_Struct( cSurface,0,SDL_FreeSurface,self_surf );
	/* The default initialize() does nothing, but may be overridden. */
	rb_obj_call_init(self,argc,argv);
	return self;
}


VALUE rbgm_surface_initialize(int argc, VALUE *argv, VALUE self)
{
	return self;
}


/* 
 *  call-seq:
 *     width
 *     w
 *
 *  Return the width (in pixels) of the surface. 
 */
VALUE rbgm_surface_get_w(VALUE self)
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return INT2NUM(surf->w);
}

/* 
 *  call-seq:
 *     height
 *     h
 *
 *  Return the height (in pixels) of the surface. 
 */
VALUE rbgm_surface_get_h(VALUE self)
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return INT2NUM(surf->h);
}

/*  call-seq:
 *    size  ->  [w,h]
 *
 *  Return the surface's width and height (in pixels) in an Array.
 */
VALUE rbgm_surface_get_size(VALUE self)
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return rb_ary_new3( 2, INT2NUM(surf->w), INT2NUM(surf->h) );
}

/*  call-seq:
 *     depth
 *
 * Return the color depth (in bits per pixel) of the surface.
 */
VALUE rbgm_surface_get_depth(VALUE self)
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return UINT2NUM(surf->format->BitsPerPixel);
}

/*  call-seq:
 *     flags
 *
 *  Return any flags the surface was initialized with 
 *  (as a bitwise OR'd integer).
 */
VALUE rbgm_surface_get_flags(VALUE self)
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return UINT2NUM(surf->flags);
}

/* 
 *  call-seq:
 *    masks  ->  [r,g,b,a]
 *
 *  Return the color masks [r,g,b,a] of the surface. Almost everyone can
 *  ignore this function. Color masks are used to separate an
 *  integer representation of a color into its seperate channels.
 */
VALUE rbgm_surface_get_masks(VALUE self)
{
	SDL_Surface *surf;
	SDL_PixelFormat *format;

	Data_Get_Struct(self, SDL_Surface, surf);
	format = surf->format;
	return rb_ary_new3(4,\
		UINT2NUM(format->Rmask),\
		UINT2NUM(format->Gmask),\
		UINT2NUM(format->Bmask),\
		UINT2NUM(format->Amask));
}

/* 
 *  call-seq:
 *     alpha
 *
 *  Return the per-surface alpha (opacity; non-transparency) of the surface.
 *  It can range from 0 (full transparent) to 255 (full opaque).
 */
VALUE rbgm_surface_get_alpha(VALUE self)
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return INT2NUM(surf->format->alpha);
}

/*
 *  call-seq:
 *     set_alpha(alpha, flags=Rubygame::SRC_ALPHA)
 *
 *  Set the per-surface alpha (opacity; non-transparency) of the surface.
 *
 *  This function takes these arguments:
 *  alpha:: requested opacity of the surface. Alpha must be from 0 
 *          (fully transparent) to 255 (fully opaque).
 *  flags:: 0 or Rubygame::SRC_ALPHA (default). Most people will want the
 *          default, in which case this argument can be omitted. For advanced
 *          users: this flag affects the surface as described in the docs for
 *          the SDL C function, SDL_SetAlpha.
 */
VALUE rbgm_surface_set_alpha(int argc, VALUE *argv, VALUE self)
{
	SDL_Surface *surf;
	Uint8 alpha;
	Uint32 flags = SDL_SRCALPHA;

	switch(argc)
	{
		case 2: flags = NUM2UINT(argv[1]);
			/* no break */
		case 1: 
		  {
			int temp;
			temp = NUM2INT(argv[0]);
			if(temp<0) alpha = 0;
			else if(temp>255) alpha = 255;
			else alpha = (Uint8) temp;
			break;
		  }
		default:
			rb_raise(rb_eArgError,\
				"Wrong number of args to set mode (%d for 1)",argc);
	}

	Data_Get_Struct(self,SDL_Surface,surf);
	if(SDL_SetAlpha(surf,flags,alpha)!=0)
		rb_raise(eSDLError,"%s",SDL_GetError());
	return self;
}

/*
 *  call-seq:
 *     colorkey  ->  [r,g,b]  or  nil
 *
 *  Return the colorkey of the surface in the form [r,g,b] (or +nil+ if there
 *  is no key). The colorkey of a surface is the exact color which will be
 *  ignored when the surface is blitted, effectively turning that color
 *  transparent. This is often used to make a blue (for example) background
 *  on an image seem transparent.
 */
VALUE rbgm_surface_get_colorkey( VALUE self )
{
	SDL_Surface *surf;
	Uint32 colorkey;
	Uint8 r,g,b;

	Data_Get_Struct(self, SDL_Surface, surf);
	colorkey = surf->format->colorkey;
	if((int *)colorkey == NULL)
		return Qnil;
	SDL_GetRGB(colorkey, surf->format, &r, &g, &b);
	return rb_ary_new3(3,UINT2NUM(r),UINT2NUM(g),UINT2NUM(b));
}

/*
 *  call-seq:
 *     set_colorkey(color,flags=0)
 *
 *  Set the colorkey of the surface. See Surface#colorkey for a description
 *  of colorkeys.
 *
 *  This method takes these arguments:
 *  color:: color to use as the key, in the form [r,g,b]. Can be +nil+ to
 *          un-set the colorkey.
 *  flags:: 0 or Rubygame::SRC_COLORKEY (default) or 
 *          Rubygame::SRC_COLORKEY|Rubygame::SDL_RLEACCEL. Most people will 
 *          want the default, in which case this argument can be omitted. For
 *          advanced users: this flag affects the surface as described in the
 *          docs for the SDL C function, SDL_SetColorkey.
 */
VALUE rbgm_surface_set_colorkey( int argc, VALUE *argv, VALUE self)
{
	SDL_Surface *surf;
	Uint32 color;
	Uint32 flag;
	Uint8 r,g,b;

	Data_Get_Struct(self, SDL_Surface, surf);
	if(argv[0] == Qnil)
	{
		flag = 0;
		color = 0;
	}
	else
	{
		if(argc > 1)
			flag = NUM2UINT(argv[1]);
		else
			flag = SDL_SRCCOLORKEY;

		r = NUM2UINT(rb_ary_entry(argv[0],0));
		g = NUM2UINT(rb_ary_entry(argv[0],1));
		b = NUM2UINT(rb_ary_entry(argv[0],2));
		//printf("RGB: %d,%d,%d  ",r,g,b);
		color = SDL_MapRGB(surf->format, r,g,b);
		//printf("colorkey: %d\n", color);
	}

	if(SDL_SetColorKey(surf,flag,color)!=0)
		rb_raise(eSDLError,"could not set colorkey: %s",SDL_GetError());
	return self;
}

/* Apparently it is not desirable to define these functions when
 * using Micrsoft Visual C.
 */
#ifndef _MSC_VER

static inline int max(int a, int b) {
	return a > b ? a : b;
}
static inline int min(int a, int b) {
	return a > b ? b : a;
}

#endif

/* 
 *  call-seq:
 *     blit(target,dest,source=nil)  ->  Rect
 *
 *  Blit (copy) all or part of the surface's image to another surface,
 *  at a given position. Returns a Rect representing the area of 
 *  +target+ which was affected by the blit.
 *
 *  This method takes these arguments:
 *  target:: the target Surface on which to paste the image.
 *  dest::   the coordinates of the top-left corner of the blit. Affects the
 *           area of +other+ the image data is /pasted/ over.
 *           Can also be a Rect or an Array larger than 2, but
 *           width and height will be ignored. 
 *  source:: a Rect representing the area of the source surface to get data
 *           from. Affects where the image data is /copied/ from.
 *           Can also be an Array of no less than 4 values. 
 */
VALUE rbgm_surface_blit(int argc, VALUE *argv, VALUE self)
{
	int left, top, right, bottom;
	int blit_x,blit_y,blit_w,blit_h;
	//int dest_x,dest_y,dest_w,dest_h;
	int src_x,src_y,src_w,src_h;
	VALUE returnrect;
	SDL_Surface *src, *dest;
	SDL_Rect *src_rect, *blit_rect;

	if(argc < 2 || argc > 3)
		rb_raise( rb_eArgError,"Wrong number of arguments to blit (%d for 2)",argc);


	Data_Get_Struct(self, SDL_Surface, src);
	Data_Get_Struct(argv[0], SDL_Surface, dest);

	blit_x = NUM2INT(rb_ary_entry(argv[1],0));
	blit_y = NUM2INT(rb_ary_entry(argv[1],1));

	/* did we get a src_rect argument or not? */
	if(argc>2 && argv[2]!=Qnil)
	{
		/* it might be good to check that it's actually a rect */
		src_x = NUM2INT(rb_ary_entry(argv[2],0));
		src_y = NUM2INT(rb_ary_entry(argv[2],1));
		src_w = NUM2INT(rb_ary_entry(argv[2],2));
		src_h = NUM2INT(rb_ary_entry(argv[2],3));
	}
	else
	{
		src_x = 0;
		src_y = 0;
		src_w = src->w;
		src_h = src->h;
	}
	src_rect = make_rect( src_x, src_y, src_w, src_h );

	/* experimental (broken) rectangle cropping code */
	/* crop if it went off left/top/right/bottom */
	//left = max(blit_x,0);
	//top = max(blit_y,0);
	//right = min(blit_x+src_w,dest->w);
	//bottom = min(blit_y+src_h,dest->h);

	left = blit_x;
	top = blit_y;
	right = blit_x+src_w;
	bottom = blit_y+src_h;
		
	//blit_w = min(blit_x+blit_w,dest->w) - max(blit_x,0);
	//blit_h = min(blit_y+blit_h,dest->h) - max(blit_y,0);
	blit_w = right - left;
	blit_h = bottom - top;
	
	blit_rect = make_rect( left, top, blit_w, blit_h );

	SDL_BlitSurface(src,src_rect,dest,blit_rect);

	returnrect = rb_funcall(cRect,rb_intern("new"),4,
	                        INT2NUM(left),INT2NUM(top),\
	                        INT2NUM(blit_w),INT2NUM(blit_h));

	free(blit_rect);
	free(src_rect);
	return returnrect;
}

/* 
 *  call-seq:
 *     fill(color,rect=nil)
 *
 *  Fill all or part of a Surface with a color.
 *
 *  This method takes these arguments:
 *  color:: color to fill with, in the form +[r,g,b]+ or +[r,g,b,a]+ (for
 *          partially transparent fills).
 *  rect::  a Rubygame::Rect representing the area of the surface to fill
 *          with color. Omit to fill the entire surface.
 */
VALUE rbgm_surface_fill( int argc, VALUE *argv, VALUE self )
{
	SDL_Surface *surf;
	SDL_Rect *rect;
	Uint32 color;
	Uint8 r,g,b,a;

	Data_Get_Struct(self, SDL_Surface, surf);

	if(argc < 1)
	{
		rb_raise(rb_eArgError,"wrong number of arguments (%d for 1 or 2)",argc);
	}

	r = NUM2UINT(rb_ary_entry(argv[0],0));
	g = NUM2UINT(rb_ary_entry(argv[0],1));
	b = NUM2UINT(rb_ary_entry(argv[0],2));
	/* if the array is larger than [R,G,B], it should be [R,G,B,A] */
	if(RARRAY(argv[0])->len > 3)
	{
		a = NUM2UINT(rb_ary_entry(argv[0],3));
		color = SDL_MapRGBA(surf->format, r,g,b,a);
	}
	else
	{
		color = SDL_MapRGB(surf->format, r,g,b);
	}

	switch(argc)
	{
		case 1: /* fill whole thing */
			SDL_FillRect(surf,NULL,color);
			break;
		case 2: /* fill a given rect */
			rect = make_rect(\
			                 NUM2INT(rb_ary_entry(argv[1],0)),\
			                 NUM2INT(rb_ary_entry(argv[1],1)),\
			                 NUM2INT(rb_ary_entry(argv[1],2)),\
			                 NUM2INT(rb_ary_entry(argv[1],3))\
			                 );
			SDL_FillRect(surf,rect,color);
			free(rect);
			break;
		default:
			rb_raise( rb_eArgError,"Wrong number of arguments to fill (%d for 1 or 2)",NUM2INT(argc));
			break;
	}
	return self;
}

/* 
 *  call-seq: 
 *     get_at(x,y)
 *
 *  Return the color [r,g,b,a] of the pixel at the given coordinate. 
 *
 *  Raises IndexError if the coordinates are out of bounds.
 */
VALUE rbgm_surface_getat( VALUE self, VALUE vx, VALUE vy )
{
	SDL_Surface *surf;
	int x, y, locked;
	Uint32 color;
	Uint8 *pixels, *pix;
	Uint8 r,g,b,a;

	Data_Get_Struct(self, SDL_Surface, surf);

	x = NUM2INT(vx);
	y = NUM2INT(vy);

	if( x < 0 || x > surf->w )
		rb_raise(rb_eIndexError,"x index out of bounds (%d, min 0, max %d)",\
			x,surf->w);
	if( y < 0 || y > surf->h )
		rb_raise(rb_eIndexError,"y index out of bounds (%d, min 0, max %d)",\
			y,surf->h);

	locked = 0;
	/* lock surface */
	if(SDL_MUSTLOCK(surf))
	{
		if(SDL_LockSurface(surf)==0)
			locked += 1;
		else
			rb_raise(eSDLError,"could not lock surface: %s",SDL_GetError());
	}

/* borrowed from pygame */
	pixels = (Uint8 *) surf->pixels;

	switch(surf->format->BytesPerPixel)
	{
		case 1:
			color = (Uint32)*((Uint8 *)(pixels + y * surf->pitch) + x);
			break;
		case 2:
			color = (Uint32)*((Uint16 *)(pixels + y * surf->pitch) + x);
			break;
		case 3:
			pix = ((Uint8 *)(pixels + y * surf->pitch) + x * 3);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			color = (pix[0]) + (pix[1]<<8) + (pix[2]<<16);
#else
			color = (pix[2]) + (pix[1]<<8) + (pix[0]<<16);
#endif
			break;
		default: /*case 4:*/
			color = *((Uint32*)(pixels + y * surf->pitch) + x);
			break;
	}

/* end borrowed from pygame */

	/* recursively unlock surface*/
	while(locked>1)
	{
		SDL_UnlockSurface(surf);
		locked -= 1;
	}

	if((int *)color == NULL)
	{
		VALUE zero = INT2NUM(0);
		return rb_ary_new3(4,zero,zero,zero,zero);
	}

	SDL_GetRGBA(color, surf->format, &r, &g, &b, &a);
	return rb_ary_new3(4,UINT2NUM(r),UINT2NUM(g),UINT2NUM(b),UINT2NUM(a));
}

/*
 *  call-seq:
 *    pixels  ->  String
 *
 *  Return a string of pixel data for the Surface. Most users will not
 *  need to use this method. If you want to convert a Surface into an
 *  OpenGL texture, pass the returned string to the TexImage2D method
 *  of the ruby-opengl library. (See samples/demo_gl_tex.rb for an example.)
 *
 *  (Please note that the dimensions of OpenGL textures must be powers of 2
 *  (e.g. 64x128, 512x512), so if you want to use a Surface as an OpenGL
 *  texture, the Surface's dimensions must also be powers of 2!)
 */
VALUE rbgm_surface_pixels( VALUE self )
{
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);
	return rb_str_new(surf->pixels, (long)surf->pitch * surf->h);
}

/*
 *  call-seq:
 *    clip  ->  Rect
 *
 *  Return the clipping area for this Surface. See also #cliprect=.
 *
 *  The clipping area of a Surface is the only part which can be drawn upon
 *  by other Surface's #blits. By default, the clipping area is the entire area
 *  of the Surface.
 */
VALUE rbgm_surface_get_clip( VALUE self )
{
	SDL_Rect rect;
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);

	SDL_GetClipRect(surf, &rect);

	return rb_funcall(cRect,rb_intern("new"),4,
	                  INT2NUM(rect.x),INT2NUM(rect.y),
	                  INT2NUM(rect.w),INT2NUM(rect.h));
}

/*
 *  call-seq:
 *    clip = Rect or nil
 *
 *  Set the current clipping area of the Surface. See also #cliprect.
 *
 *  The clipping area of a Surface is the only part which can be drawn upon
 *  by other Surface's #blits. The clipping area will be clipped to the edges
 *  of the surface so that the clipping area for a Surface can never fall
 *  outside the edges of the Surface.
 *
 *  By default, the clipping area is the entire area of the Surface.
 *  You may set clip to +nil+, which will reset the clipping area to cover
 *  the entire Surface.
 */
VALUE rbgm_surface_set_clip( VALUE self, VALUE clip )
{
	SDL_Rect *rect;
	SDL_Surface *surf;
	Data_Get_Struct(self, SDL_Surface, surf);


	if(clip == Qnil)
	{
		SDL_SetClipRect(surf,NULL);
	}
	else
	{ 
		rect = make_rect(\
		                 NUM2INT(rb_ary_entry(clip,0)),\
		                 NUM2INT(rb_ary_entry(clip,1)),\
		                 NUM2INT(rb_ary_entry(clip,2)),\
		                 NUM2INT(rb_ary_entry(clip,3))\
		                 );

		SDL_SetClipRect(surf,rect);
	}

	return self;
}

/* 
 *  call-seq:
 *    convert( other=nil, flags=nil )  ->  Surface
 *
 *  Copies the Surface to a new Surface with the pixel format of another
 *  Surface, for fast blitting. May raise SDLError if a problem occurs.
 *
 *  This method takes these arguments:
 *  - other::  The Surface to match pixel format against. If +nil+, the
 *             display surface (i.e. Screen) is used, if available; if no
 *             display surface is available, raises SDLError.
 *  - flags::  An array of flags to pass when the new Surface is created.
 *             See Surface#new.
 *
 */
VALUE rbgm_surface_convert(int argc, VALUE *argv, VALUE self)
{
	SDL_Surface *surf, *othersurf, *newsurf;
  Uint32 flags = 0;

	Data_Get_Struct(self, SDL_Surface, surf);

	if(argc>0 && argv[0]!=Qnil)
  {
    Data_Get_Struct(argv[0], SDL_Surface, othersurf);
  }
  else
  {
    othersurf = SDL_GetVideoSurface();
    if( othersurf == NULL )
    {
      rb_raise(eSDLError, "Cannot convert Surface with no target given and no Screen made: %s", SDL_GetError());
    }
  }

	if(argc>1 && argv[1]!=Qnil)
  {
    flags = NUM2UINT(argv[1]);
  }

  newsurf = SDL_ConvertSurface( surf, othersurf->format, flags );

  if( newsurf == NULL )
  {
    rb_raise(eSDLError,\
             "Could not convert the Surface: %s",\
             SDL_GetError());
  }

  return Data_Wrap_Struct( cSurface,0,SDL_FreeSurface,newsurf );  
}

/* 
 *  call-seq: 
 *    to_display()  ->  Surface
 *
 *  Copies the Surface to a new Surface with the pixel format of the display,
 *  suitable for fast blitting to the display surface (i.e. Screen).
 *  May raise SDLError if a problem occurs.
 *
 *  If you want to take advantage of hardware colorkey or alpha blit
 *  acceleration, you should set the colorkey and alpha value before calling
 *  this function. 
 *
 */
VALUE rbgm_surface_dispform(VALUE self)
{
	SDL_Surface *surf, *newsurf;
	Data_Get_Struct(self, SDL_Surface, surf);

  newsurf = SDL_DisplayFormat( surf );

  if( newsurf == NULL )
  {
    rb_raise(eSDLError,\
             "Could not convert the Surface to display format: %s",\
             SDL_GetError());
  }

  return Data_Wrap_Struct( cSurface,0,SDL_FreeSurface,newsurf );  
}

/* 
 *  call-seq: 
 *    to_display_alpha()  ->  Surface
 *
 *  Like #to_display except the Surface has an extra channel for alpha (i.e.
 *  opacity). May raise SDLError if a problem occurs.
 *
 *  This function can be used to convert a colorkey to an alpha channel, if the
 *  SRCCOLORKEY flag is set on the surface. The generated surface will then be
 *  transparent (alpha=0) where the pixels match the colorkey, and opaque
 *  (alpha=255) elsewhere. 
 */
VALUE rbgm_surface_dispformalpha(VALUE self)
{
	SDL_Surface *surf, *newsurf;
	Data_Get_Struct(self, SDL_Surface, surf);

  newsurf = SDL_DisplayFormatAlpha( surf );

  if( newsurf == NULL )
  {
    rb_raise(eSDLError,\
             "Could not convert the Surface to display format with alpha channel: %s",\
             SDL_GetError());
  }

  return Data_Wrap_Struct( cSurface,0,SDL_FreeSurface,newsurf );  
}


/* 
 *  call-seq:
 *    savebmp( filename )  ->  nil
 *
 *  Save the Surface as a Windows Bitmap (BMP) file with the given filename.
 */
VALUE rbgm_image_savebmp( VALUE self, VALUE filename )
{
	char *name;
	SDL_Surface *surf;

	name = StringValuePtr(filename);
	Data_Get_Struct(self,SDL_Surface,surf);
	if(SDL_SaveBMP(surf,name)!=0)
	{
		rb_raise(eSDLError,\
			"Couldn't save surface to file %s: %s",name,SDL_GetError());
	}
	return Qnil;
}


/* --
 * Borrowed from Pygame:
 * ++
 */
static SDL_Surface* newsurf_fromsurf(SDL_Surface* surf, int width, int height)
{
  SDL_Surface* newsurf;

  if(surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4)
    rb_raise(eSDLError,"unsupported Surface bit depth for transform");

  newsurf = SDL_CreateRGBSurface(surf->flags, width, height, surf->format->BitsPerPixel,
        surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
  if(!newsurf)
    rb_raise(eSDLError,"%s",SDL_GetError());

  /* Copy palette, colorkey, etc info */
  if(surf->format->BytesPerPixel==1 && surf->format->palette)
    SDL_SetColors(newsurf, surf->format->palette->colors, 0, surf->format->palette->ncolors);
  if(surf->flags & SDL_SRCCOLORKEY)
    SDL_SetColorKey(newsurf, (surf->flags&SDL_RLEACCEL)|SDL_SRCCOLORKEY, surf->format->colorkey);

  return newsurf;
}

/* 
 * call-seq:
 *    flip(horz, vert)  ->  Surface
 * 
 *  Flips the source surface horizontally (if +horz+ is true), vertically
 *  (if +vert+ is true), or both (if both are true). This operation is
 *  non-destructive; the original image can be perfectly reconstructed by
 *  flipping the resultant image again.
 *
 *  This operation does NOT require SDL_gfx.
 *
 *  A similar effect can (supposedly) be achieved by giving X or Y zoom
 *  factors of -1 to #rotozoom (only if compiled with SDL_gfx 2.0.13 or
 *  greater). Your mileage may vary.
 */
VALUE rbgm_transform_flip(VALUE self, VALUE vhorz, VALUE vvert)
{
  SDL_Surface *surf, *newsurf;
  int xaxis, yaxis;

  int loopx, loopy;
  int pixsize, srcpitch, dstpitch;
  Uint8 *srcpix, *dstpix;

  xaxis = RTEST(vhorz);
  yaxis = RTEST(vvert);

  Data_Get_Struct(self,SDL_Surface,surf);

  /* Borrowed from Pygame: */
  newsurf = newsurf_fromsurf(surf, surf->w, surf->h);
  if(!newsurf)
    return Qnil;

  pixsize = surf->format->BytesPerPixel;
  srcpitch = surf->pitch;
  dstpitch = newsurf->pitch;

  SDL_LockSurface(newsurf);

  srcpix = (Uint8*)surf->pixels;
  dstpix = (Uint8*)newsurf->pixels;

  if(!xaxis)
  {
    if(!yaxis)
    {
      for(loopy = 0; loopy < surf->h; ++loopy)
        memcpy(dstpix+loopy*dstpitch, srcpix+loopy*srcpitch, surf->w*surf->format->BytesPerPixel);
    }
    else
    {
      for(loopy = 0; loopy < surf->h; ++loopy)
        memcpy(dstpix+loopy*dstpitch, srcpix+(surf->h-1-loopy)*srcpitch, surf->w*surf->format->BytesPerPixel);
    }
  }
  else /*if (xaxis)*/
  {
    if(yaxis)
    {
      switch(surf->format->BytesPerPixel)
      {
      case 1:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint8* dst = (Uint8*)(dstpix+loopy*dstpitch);
          Uint8* src = ((Uint8*)(srcpix+(surf->h-1-loopy)*srcpitch)) + surf->w - 1;
          for(loopx = 0; loopx < surf->w; ++loopx)
            *dst++ = *src--;
        }break;
      case 2:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint16* dst = (Uint16*)(dstpix+loopy*dstpitch);
          Uint16* src = ((Uint16*)(srcpix+(surf->h-1-loopy)*srcpitch)) + surf->w - 1;
          for(loopx = 0; loopx < surf->w; ++loopx)
            *dst++ = *src--;
        }break;
      case 4:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint32* dst = (Uint32*)(dstpix+loopy*dstpitch);
          Uint32* src = ((Uint32*)(srcpix+(surf->h-1-loopy)*srcpitch)) + surf->w - 1;
          for(loopx = 0; loopx < surf->w; ++loopx)
            *dst++ = *src--;
        }break;
      case 3:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint8* dst = (Uint8*)(dstpix+loopy*dstpitch);
          Uint8* src = ((Uint8*)(srcpix+(surf->h-1-loopy)*srcpitch)) + surf->w*3 - 3;
          for(loopx = 0; loopx < surf->w; ++loopx)
          {
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
            dst += 3;
            src -= 3;
          }
        }break;
      }
    }
    else
    {
      switch(surf->format->BytesPerPixel)
      {
      case 1:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint8* dst = (Uint8*)(dstpix+loopy*dstpitch);
          Uint8* src = ((Uint8*)(srcpix+loopy*srcpitch)) + surf->w - 1;
          for(loopx = 0; loopx < surf->w; ++loopx)
            *dst++ = *src--;
        }break;
      case 2:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint16* dst = (Uint16*)(dstpix+loopy*dstpitch);
          Uint16* src = ((Uint16*)(srcpix+loopy*srcpitch)) + surf->w - 1;
          for(loopx = 0; loopx < surf->w; ++loopx)
            *dst++ = *src--;
        }break;
      case 4:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint32* dst = (Uint32*)(dstpix+loopy*dstpitch);
          Uint32* src = ((Uint32*)(srcpix+loopy*srcpitch)) + surf->w - 1;
          for(loopx = 0; loopx < surf->w; ++loopx)
            *dst++ = *src--;
        }break;
      case 3:
        for(loopy = 0; loopy < surf->h; ++loopy) {
          Uint8* dst = (Uint8*)(dstpix+loopy*dstpitch);
          Uint8* src = ((Uint8*)(srcpix+loopy*srcpitch)) + surf->w*3 - 3;
          for(loopx = 0; loopx < surf->w; ++loopx)
          {
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
            dst += 3;
            src -= 3;
          }
        }break;
      }
    }
  }

  SDL_UnlockSurface(newsurf);
  /* Thanks, Pygame :) */


  if(newsurf == NULL)
    rb_raise(eSDLError,"Could not flip surface: %s",SDL_GetError());
  return Data_Wrap_Struct(cSurface,0,SDL_FreeSurface,newsurf);
}

/* 
 *  Document-class: Rubygame::Surface
 */
void Rubygame_Init_Surface()
{

#if 0
	mRubygame = rb_define_module("Rubygame");
	cSurface = rb_define_class_under(mRubygame,"Surface",rb_cObject);
#endif

	rb_define_singleton_method(cSurface,"new",rbgm_surface_new,-1);
	rb_define_method(cSurface,"initialize",rbgm_surface_initialize,-1);
	rb_define_method(cSurface,"w",rbgm_surface_get_w,0);
	rb_define_alias(cSurface,"width","w");
	rb_define_method(cSurface,"h",rbgm_surface_get_h,0);
	rb_define_alias(cSurface,"height","h");
	rb_define_method(cSurface,"size",rbgm_surface_get_size,0);
	rb_define_method(cSurface,"depth",rbgm_surface_get_depth,0);
	rb_define_method(cSurface,"flags",rbgm_surface_get_flags,0);
	rb_define_method(cSurface,"masks",rbgm_surface_get_masks,0);
	rb_define_method(cSurface,"alpha",rbgm_surface_get_alpha,0);
	rb_define_method(cSurface,"set_alpha",rbgm_surface_set_alpha,-1);
	rb_define_method(cSurface,"colorkey",rbgm_surface_get_colorkey,0);
	rb_define_method(cSurface,"set_colorkey",rbgm_surface_set_colorkey,-1);
	rb_define_method(cSurface,"blit",rbgm_surface_blit,-1);
	rb_define_method(cSurface,"fill",rbgm_surface_fill,-1);
	rb_define_method(cSurface,"get_at",rbgm_surface_getat,2);
	rb_define_method(cSurface,"pixels",rbgm_surface_pixels,0);
	rb_define_method(cSurface,"clip",rbgm_surface_get_clip,0);
	rb_define_method(cSurface,"clip=",rbgm_surface_set_clip,1);
	rb_define_method(cSurface,"convert",rbgm_surface_convert,-1);
	rb_define_method(cSurface,"to_display",rbgm_surface_dispform,0);
	rb_define_method(cSurface,"to_display_alpha",rbgm_surface_dispformalpha,0);
	rb_define_method(cSurface,"savebmp",rbgm_image_savebmp,1);
	rb_define_method(cSurface,"flip",rbgm_transform_flip,2);

	
	/* Surface initialization flags */
	rb_define_const(mRubygame,"SWSURFACE",UINT2NUM(SDL_SWSURFACE));
	rb_define_const(mRubygame,"HWSURFACE",UINT2NUM(SDL_HWSURFACE));
	rb_define_const(mRubygame,"ASYNCBLIT",UINT2NUM(SDL_ASYNCBLIT));
	rb_define_const(mRubygame,"ANYFORMAT",UINT2NUM(SDL_ANYFORMAT));
	rb_define_const(mRubygame,"HWPALETTE",UINT2NUM(SDL_HWPALETTE));
	rb_define_const(mRubygame,"HWACCEL",UINT2NUM(SDL_HWACCEL));
	rb_define_const(mRubygame,"SRCCOLORKEY",UINT2NUM(SDL_SRCCOLORKEY));
	rb_define_const(mRubygame,"RLEACCELOK",UINT2NUM(SDL_RLEACCELOK));
	rb_define_const(mRubygame,"RLEACCEL",UINT2NUM(SDL_RLEACCEL));
	rb_define_const(mRubygame,"SRCALPHA",UINT2NUM(SDL_SRCALPHA));
	rb_define_const(mRubygame,"PREALLOC",UINT2NUM(SDL_PREALLOC));

}
