#ifndef GRAPHICS_H
#define GRAPHICS_H

// Some design ramblings to see how some things may fit together.

#include <QuickVec.h>
#include <Matrix.h>
#include <Scale9.h>

typedef unsigned int uint32;


enum GraphicsAPIType { gatBase, gatInternal,  gatQuartz, gatCairo, gatOpenGL, gatOpenGLES };

enum SurfaceAPIType  { satInternal, satSDL, satCairo };

enum PixelFormat
{
   pfXRGB = 0x00,
   pfARGB = 0x01,
   pfXBGR = 0x02,
   pfABGR = 0x03,

   pfHasAlpha = 0x01,
   pfBGROrder = 0x02,
};



// --- Graphics Data -------------------------------------------------------

// These classes match the flash API very closely...
class GraphicsEndFill;
class GraphicsSolidFill;
class GraphicsGradientFill;
class GraphicsBitmapFill;

class GraphicsPath;
class GraphicsTrianglePath;

class GraphicsStroke;

class Surface;

// Don't know if these belong in the c++ level?
class IGraphicsFill;
class IGraphicsPath;
class IGraphicsStroke;


enum GraphicsDataType
{
   gdtUnknown, gdtEndFill, gdtSolidFill, gdtGradientFill, gdtBitmapFill,
   gdtPath, gdtTrianglePath, gdtStroke
};


class IGraphicsData
{
public:
   IGraphicsData() : mRefCount(0) { };

   IGraphicsData *IncRef() { mRefCount++; return this; }

   void DecRef() { mRefCount--; if (mRefCount<=0) delete this; }

   virtual GraphicsDataType GetType() { return gdtUnknown; }
   virtual GraphicsAPIType  GetAPI() { return gatBase; }

   virtual IGraphicsFill   *AsIFill() { return 0; }
   virtual IGraphicsPath   *AsIPath() { return 0; }
   virtual IGraphicsStroke   *AsIStroke() { return 0; }

   virtual GraphicsEndFill      *AsEndFill() { return 0; }
   virtual GraphicsSolidFill    *AsSolidFill() { return 0; }
   virtual GraphicsGradientFill *AsGradientFill() { return 0; }
   virtual GraphicsBitmapFill   *AsBitmapFill() { return 0; }

   virtual GraphicsStroke         *AsStroke() { return 0; }

   virtual GraphicsPath           *AsPath() { return 0; }
   virtual GraphicsTrianglePath   *AsTrianglePath() { return 0; }


protected:
   virtual ~IGraphicsData() { }
private:
   IGraphicsData(const IGraphicsData &inRHS);
   void operator=(const IGraphicsData &inRHS);

   int     mRefCount;
};


class IGraphicsFill : public IGraphicsData
{
   virtual IGraphicsFill *AsIFill() { return this; }

protected:
   virtual ~IGraphicsFill() { };
};


class GraphicsEndFill : public IGraphicsFill
{
public:
   GraphicsDataType GetType() { return gdtEndFill; }
   GraphicsEndFill   *AsEndFill() { return this; }
};

class GraphicsSolidFill : public IGraphicsFill
{
public:
	GraphicsSolidFill(int inRGB, float inAlpha) : rgb(inRGB), alpha(inAlpha) { }
   GraphicsDataType GetType() { return gdtSolidFill; }
   GraphicsSolidFill   *AsSolidFill() { return this; }

   float alpha;
   int   rgb;
};


struct GradStop
{
   float  mAlpha;
   int    mRGB;
};

enum InterpolationMethod {  imLinearRGB, imRGB };
enum SpreadMethod {  smPad, smReflect, smRepeat };

class GraphicsGradientFill : public IGraphicsFill
{
public:
   GraphicsDataType GetType() { return gdtGradientFill; }
   GraphicsGradientFill   *AsGradientFill() { return this; }

   QuickVec<GradStop>  mStops;

   double              focalPointRatio;
   Matrix              matrix;
   InterpolationMethod interpolationMethod;
   SpreadMethod        spreadMethod;
   bool                isLinear;
};


class GraphicsBitmapFill : public IGraphicsFill
{
public:
   ~GraphicsBitmapFill();

   GraphicsDataType GetType() { return gdtBitmapFill; }
   GraphicsBitmapFill   *AsBitmapFill() { return this; }

   Surface             *bitmapData;
   Matrix              matrix;
   bool                repeat;
   bool                smooth;
};

class IGraphicsStroke : public IGraphicsData
{
public:
   IGraphicsStroke *AsIStroke() { return this; }
};

enum StrokeCaps { scNone, scRound, scSquare };
enum StrokeJoints { sjMiter, sjRound, sjBevel };
enum StrokeScaleMode { ssmNormal, ssmNone, ssmVertical, ssmHorizontal };

class GraphicsStroke : public IGraphicsStroke
{
public:
   ~GraphicsStroke();

   GraphicsStroke *AsStroke() { return this; }

	bool IsClear() { return false; }

   StrokeCaps      caps;
   IGraphicsFill   *fill;
   StrokeJoints    joints;
   double          miterLimit;
   bool            pixelHinting;
   StrokeScaleMode scaleMode;
   double          thickness;
};


class IGraphicsPath : public IGraphicsData
{
public:
   IGraphicsPath *AsIPath() { return this; }
};

enum PathCommand
{
   pcNoOp  = 0,
   pcMoveTo  = 1,
   pcLineTo = 2,
   pcCurveTo =  3,
   pcWideMoveTo = 4,
   pcWideLineTo = 5,

	// This one is added to the LineData to provide the direction of
	//  the first line segment when closing a line.
   pcCloseDirection = 6,
};

enum WindingRule { wrOddEven, wrNonZero };


class GraphicsPath : public IGraphicsPath
{
public:
   GraphicsPath *AsPath() { return this; }

	GraphicsPath() : winding(wrOddEven) { }
   QuickVec<unsigned char> command;
   QuickVec<float>         data;
   WindingRule             winding;

	void curveTo(float controlX, float controlY, float anchorX, float anchorY);
	void lineTo(float x, float y);
	void moveTo(float x, float y);
	void wideLineTo(float x, float y);
	void wideMoveTo(float x, float y);
};


enum TriangleCulling { tcNegative = -1, tcNone = 0, tcPositive = 1};

class GraphicsTrianglePath : public IGraphicsPath
{
public:
   TriangleCulling   culling;
   QuickVec<int>     indices;
   QuickVec<double>  uvtData;
   QuickVec<double>  uvtVertices;
   int               mUVTDim;
};

struct IRenderData
{
public:
   virtual ~IRenderData() { }
	virtual struct SolidData *AsSolid() { return 0; }
	virtual struct LineData *AsLine() { return 0; }
	virtual struct TriangleData *AsTriangles() { return 0; }
};

struct SolidData : IRenderData
{
	SolidData(IGraphicsFill *inFill) : mFill(inFill) { }
	SolidData *AsSolid() { return this; }
	void Add(GraphicsPath *inPath);
	void Close();

   IGraphicsFill           *mFill;
   QuickVec<unsigned char> command;
   QuickVec<float>        data;
};

struct LineData : IRenderData
{
	LineData(GraphicsStroke *inStroke=0) : mStroke(inStroke) { }
	LineData *AsLine() { return this; }
	void Add(GraphicsPath *inPath);

   GraphicsStroke         *mStroke;
   QuickVec<unsigned char> command;
   QuickVec<float>        data;
};

struct TriangleData : IRenderData
{
	TriangleData *AsTriangles() { return this; }
   IGraphicsFill           *mFill;
   IGraphicsStroke         *mStroke;
   TriangleData            *mTriangles;
};



// ----------------------------------------------------------------------


// Blender = blend mode + (colour transform + alpha)

enum BlendMode { bmNormal, nmAdd };

struct Rect
{
   Rect(int inW=0,int inH=0) : x(0), y(0), w(inW), h(inH) { } 
   Rect(int inX,int inY,int inW,int inH) : x(inX), y(inY), w(inW), h(inH) { } 

	Rect Intersect(const Rect &inOther) const;
	int x1() const { return x+w; }
	int y1() const { return y+h; }

   int x,y;
   int w,h;
};

class ColorTransform
{
   double redScale, redOffset;
   double greenScale, greenOffset;
   double blueScale, blueOffset;
   double alphaScale, alphaOffset;
};

struct Mask
{
   // ??
};

struct Transform
{
	Transform();

	bool           DifferentSpace(const Transform &inRHS) const;
	UserPoint      Apply(float inX, float inY) const;

   Matrix3D       mMatrix3D;
   Matrix         mMatrix;
   Scale9         mScale9;

   double         mAlpha;
   BlendMode      mBlendMode;
   ColorTransform mTransform;

   Rect           mClipRect;
   Mask           mMask;
};

class IRenderCache
{
public:
	virtual void Destroy() { delete this; }

protected:
   IRenderCache() { }
   virtual ~IRenderCache() { }
};


typedef QuickVec<IRenderData *> RenderData;

class Graphics
{
public:
   Graphics();
   ~Graphics();


   void drawGraphicsData(IGraphicsData **graphicsData,int inN);
   void beginFill(unsigned int color, float alpha = 1.0);

   void lineTo(float x, float y);
   void moveTo(float x, float y);


	const RenderData &CreateRenderData();
   IRenderCache  *mSoftwareCache;
   IRenderCache  *mHardwareCache;

private:
   QuickVec<IGraphicsData *> mItems;
	RenderData                mRenderData;

	void Add(IGraphicsData *inData);
	void Add(IRenderData *inData);
	GraphicsPath *GetLastPath();

   int mLastConvertedItem;

private:
   // Rule of 3 - we must manually delete the mItems...
   Graphics(const Graphics &inRHS);
   void operator=(const Graphics &inRHS);
};

struct Tile
{
   Surface *mData;
   Rect     mRect;
	double   mX0;
	double   mY0;
};

typedef char *String;

class NativeFont;

class TextData
{
   String     mText;
   NativeFont *mFont;
   uint32     mColour;
   double     mSize;
   double     mX;
   double     mY;
};


typedef QuickVec<TextData> TextList;

class IRenderTarget
{
public:
   virtual int  Width() const =0;
   virtual int  Height() const =0;

   virtual void ViewPort(int inOX,int inOY, int inW,int inH)=0;
   virtual void BeginRender()=0;
   virtual void Render(Graphics &inDisplayList, const Transform &inTransform)=0;
   virtual void Render(TextList &inTextList, const Transform &inTransform)=0;
   virtual void Blit(Tile &inBitmap, int inOX, int inOY, double inScale, int Rotation)=0;
   virtual void EndRender() = 0;
};

enum EventType
{
   etUnknown,
   etClose,
   etResize,
   etMouseMove,
   etMouseClick,
   etTimer,
   etRedraw,
   etNextFrame,
};


struct Event
{
	Event(EventType inType=etUnknown) :
		  mType(inType), mWinX(0), mWinY(0), mValue(0), mModState(0)
	{
	}

   EventType mType;
   int       mWinX,mWinY;
   int       mValue;
   int       mModState;
};

typedef void (*EventHandler)(Event &ioEvent, void *inUserData);

class DisplayObject
{
public:

};


class DisplayObjectContainer : public DisplayObject
{
public:

};

class Stage : public DisplayObjectContainer
{
public:
   virtual void Flip() = 0;
   virtual void GetMouse() = 0;
   virtual void SetEventHandler(EventHandler inHander,void *inUserData) = 0;
	virtual IRenderTarget *GetRenderTarget() = 0;
};


class Frame
{
public:
   virtual void SetTitle() = 0;
   virtual void SetIcon() = 0;
   virtual Stage *GetStage() = 0;
};

enum WindowFlags
{
   wfFullScreen = 0x00000001,
   wfBorderless = 0x00000002,
   wfResizable  = 0x00000004,
   wfOpenGL     = 0x00000008,
};

Frame *CreateMainFrame(int inWidth,int inHeight,unsigned int inFlags, String inTitle );
void MainLoop();
void TerminateMainLoop();

#ifdef _WIN32
//Frame *CreateNativeFrame(HWND inParent);
#endif


// ---- Surface API --------------


struct SurfaceData
{
   unsigned char *data;
   int  width;
   int  height;
   int  stride;
};

enum
{
   surfLockRead = 0x0001,
   surfLockWrite = 0x0002,
};

class Surface
{
public:
   Surface();
   virtual ~Surface();

   virtual int Width() const =0;
   virtual int Height() const =0;
   virtual PixelFormat Format()  const = 0;

   virtual void Blit(Surface *inSrc, const Rect &inSrcRect,int inDX, int inDY)=0;
   virtual SurfaceData Lock(const Rect &inRect,uint32 inFlags)=0;
   virtual void Unlock()=0;

   virtual IRenderCache *GetTexture() { return mTexture; }
   virtual void SetTexture(IRenderCache *inTexture);

protected:
   IRenderCache *mTexture;
};

class SimpleSurface : public Surface
{
public:
   SimpleSurface(int inWidth,int inHeight,PixelFormat inPixelFormat,int inByteAlign=4);
   ~SimpleSurface();

   int Width() const  { return mWidth; }
   int Height() const  { return mHeight; }
   PixelFormat Format() const  { return mPixelFormat; }

   void Blit(Surface *inSrc, const Rect &inSrcRect,int inDX, int inDY);
   SurfaceData Lock(const Rect &inRect,uint32 inFlags);
   void Unlock();

protected:
   int           mWidth;
   int           mHeight;
   PixelFormat   mPixelFormat;
   int           mStride;
   unsigned char *mBase;

private:
   SimpleSurface(const SimpleSurface &inRHS);
   void operator=(const SimpleSurface &inRHS);
};

IRenderTarget *CreateSurfaceRenderTarget(Surface *inSurface);



#endif
