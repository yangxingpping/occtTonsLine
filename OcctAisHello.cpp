
#include <windows.h>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewController.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OSD.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include <WNT_WClass.hxx>
#include <WNT_Window.hxx>
#include <V3d_RectangularGrid.hxx>
#include <Prs3d_Presentation.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_Text.hxx>

#ifdef _MSC_VER
#pragma comment(lib, "TKOpenGl.lib")
#pragma comment(lib, "TKV3d.lib")
#pragma comment(lib, "TKPrim.lib")
#pragma comment(lib, "TKTopAlgo.lib")
#pragma comment(lib, "TKBRep.lib")
#pragma comment(lib, "TKService.lib")
#pragma comment(lib, "TKMath.lib")
#pragma comment(lib, "TKernel.lib")
#endif


//! Sample single-window viewer class.
class OcctAisHello : public AIS_ViewController
{
public:
	//! Main constructor.
	OcctAisHello()
	{
		// graphic driver setup
		Handle(Aspect_DisplayConnection) aDisplay = new Aspect_DisplayConnection();
		Handle(Graphic3d_GraphicDriver) aDriver = new OpenGl_GraphicDriver(aDisplay);

		// viewer setup
		Handle(V3d_Viewer) aViewer = new V3d_Viewer(aDriver);
		aViewer->SetDefaultLights();
		aViewer->SetLightOn();

		Handle(Prs3d_Presentation) ap = new Prs3d_Presentation(aViewer->StructureManager());

		Handle(Graphic3d_Group) group = ap->NewGroup();
		Handle(Graphic3d_ArrayOfSegments) segments = new Graphic3d_ArrayOfSegments(40000, 0, Standard_True);

		for (int i = 0; i < 100; ++i)
		{
			segments->AddVertex(gp_Pnt(0, 0, i));
			segments->AddVertex(gp_Pnt(100, 0, i));

			segments->AddVertex(gp_Pnt(0, 0, i));
			segments->AddVertex(gp_Pnt(0, 100, i));
		}
		
		Handle(Graphic3d_AspectLine3d) aLineAspect = new Graphic3d_AspectLine3d(Quantity_NOC_PINK4, Aspect_TOL_SOLID, 0.1);
		group->SetPrimitivesAspect(aLineAspect);
		group->AddPrimitiveArray(segments);

		auto aText = new Graphic3d_Text(1.0f / 81.0f);
		aText->SetText("Y");
		aText->SetPosition(gp_Pnt(0, 20, 0));
		group->AddText(aText);

		// view setup
		myView = new V3d_View(aViewer);
		const TCollection_AsciiString aClassName("MyWinClass");
		Handle(WNT_WClass) aWinClass = new WNT_WClass(aClassName.ToCString(), &windowProcWrapper,
			CS_VREDRAW | CS_HREDRAW, 0, 0,
			::LoadCursor(NULL, IDC_ARROW));
		Handle(WNT_Window) aWindow = new WNT_Window("OCCT Viewer", aWinClass, WS_OVERLAPPEDWINDOW,
			100, 100, 512, 512, Quantity_NOC_BLACK);
		::SetWindowLongPtrW((HWND)aWindow->NativeHandle(), GWLP_USERDATA, (LONG_PTR)this);

		myView->SetWindow(aWindow);
		myView->SetBackgroundColor(Quantity_NOC_GRAY50);
		myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.1);
		myView->ChangeRenderingParams().RenderResolutionScale = 2.0f;
		

		// interactive context and demo scene
		myContext = new AIS_InteractiveContext(aViewer);

		TopoDS_Shape aShape = BRepPrimAPI_MakeBox(100, 100, 100).Solid();
		Handle(AIS_InteractiveObject) aShapePrs = new AIS_Shape(aShape);
		//myContext->Display(aShapePrs, AIS_Shaded, 0, false);
		myView->FitAll(0.01, false);

		ap->Display();

		aWindow->Map();
		myView->Redraw();
	}

	//! Return context.
	const Handle(AIS_InteractiveContext)& Context() const { return myContext; }

	//! Return view.
	const Handle(V3d_View)& View() const { return myView; }

private:
	//! Handle expose event.
	virtual void ProcessExpose() override
	{
		if (!myView.IsNull())
		{
			FlushViewEvents(myContext, myView, true);
		}
	}

	//! Handle window resize event.
	virtual void ProcessConfigure(bool theIsResized) override
	{
		if (!myView.IsNull() && theIsResized && !myView->Window().IsNull())
		{
			myView->Window()->DoResize();
			myView->MustBeResized();
			myView->Invalidate();
			FlushViewEvents(myContext, myView, true);
		}
	}

	//! Handle input.
	virtual void ProcessInput() override
	{
		if (!myView.IsNull())
		{
			ProcessExpose();
		}
	}

	//! Window message handler.
	static LRESULT WINAPI windowProcWrapper(HWND theWnd, UINT theMsg, WPARAM theParamW, LPARAM theParamL)
	{
		if (theMsg == WM_CLOSE)
		{
			exit(0);
			return 0;
		}

		if (OcctAisHello* aThis = (OcctAisHello*)::GetWindowLongPtrW(theWnd, GWLP_USERDATA))
		{
			WNT_Window* aWindow = dynamic_cast<WNT_Window*>(aThis->myView->Window().get());
			MSG aMsg = { theWnd, theMsg, theParamW, theParamL };
			if (aWindow->ProcessMessage(*aThis, aMsg))
			{
				return 0;
			}
		}
		return ::DefWindowProcW(theWnd, theMsg, theParamW, theParamL);
	}
private:
	Handle(AIS_InteractiveContext) myContext;
	Handle(V3d_View) myView;
};

int main()
{
	OSD::SetSignal(false);
	OcctAisHello aViewer;

	for (;;)
	{
		MSG aMsg = {};
		if (GetMessageW(&aMsg, NULL, 0, 0) <= 0)
		{
			return 0;
		}
		TranslateMessage(&aMsg);
		DispatchMessageW(&aMsg);
	}

	return 0;
}
