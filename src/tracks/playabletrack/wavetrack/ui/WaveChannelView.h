/**********************************************************************

Audacity: A Digital Audio Editor

WaveChannelView.h

Paul Licameli split from class WaveTrack

**********************************************************************/

#ifndef __AUDACITY_WAVE_TRACK_VIEW__
#define __AUDACITY_WAVE_TRACK_VIEW__

#include "../../../ui/CommonChannelView.h"
#include "ClientData.h"
#include "SampleCount.h"
namespace WaveChannelViewConstants{ enum Display : int; }
struct WaveChannelSubViewType;

class CutlineHandle;
class TranslatableString;
class SampleTrack;
class WaveTrack;
class WaveChannelView;
class WaveClip;
class WaveClipTrimHandle;
class ZoomInfo;


class TrackPanelResizeHandle;
class WaveTrackAffordanceHandle;

class SubViewCloseHandle;
class SubViewAdjustHandle;
class SubViewRearrangeHandle;

class wxDC;

class AUDACITY_DLL_API WaveChannelSubView : public CommonChannelView
{
public:

   using Display = WaveChannelViewConstants::Display;
   using Type = WaveChannelSubViewType;

   explicit
   WaveChannelSubView(WaveChannelView &waveChannelView);
   
   virtual const Type &SubViewType() const = 0;

   // For undo and redo purpose
   // Empty abstract method to be inherited, for copying the spectral data in SpectrumSubView
   virtual void CopyToSubView(WaveChannelSubView *destSubView) const;

   std::pair<
      bool, // if true, hit-testing is finished
      std::vector<UIHandlePtr>
   > DoDetailedHitTest(
      const TrackPanelMouseState &state,
      const AudacityProject *pProject, int currentTool, bool bMultiTool,
      const std::shared_ptr<WaveTrack> &wt );
   
protected:
   static void DrawBoldBoundaries(
      TrackPanelDrawingContext &context, const WaveTrack *track,
      const wxRect &rect );

   std::weak_ptr<WaveChannelView> GetWaveChannelView() const;

   std::vector<MenuItem> GetMenuItems(
      const wxRect &rect, const wxPoint *pPosition, AudacityProject *pProject )
   override;

private:
   std::weak_ptr<SubViewCloseHandle> mCloseHandle;
   std::weak_ptr<TrackPanelResizeHandle> mResizeHandle;
   std::weak_ptr<SubViewAdjustHandle> mAdjustHandle;
   std::weak_ptr<SubViewRearrangeHandle> mRearrangeHandle;
   std::weak_ptr<WaveClipTrimHandle> mClipTrimHandle;
   std::weak_ptr<CutlineHandle> mCutlineHandle;
   std::weak_ptr<WaveChannelView> mwWaveChannelView;
};

struct WaveChannelSubViewPlacement {
   int index;
   float fraction;
};
using WaveChannelSubViewPlacements = std::vector<WaveChannelSubViewPlacement>;

class WaveChannelView;
using WaveChannelSubViews = ClientData::Site<
   WaveChannelView, WaveChannelSubView, ClientData::SkipCopying, std::shared_ptr
>;

class AUDACITY_DLL_API WaveChannelView final
   : public CommonChannelView
   , public WaveChannelSubViews
{
   WaveChannelView(const WaveChannelView&) = delete;
   WaveChannelView &operator=(const WaveChannelView&) = delete;

public:
   static constexpr int kChannelSeparatorThickness{ 8 };

   using Display = WaveChannelViewConstants::Display;

   static WaveChannelView &Get(WaveTrack &track);
   static const WaveChannelView &Get(const WaveTrack &track);
   static WaveChannelView *Find(WaveTrack *pTrack);
   static const WaveChannelView *Find(const WaveTrack *pTrack);

   //! Construct a view of one channel
   /*!
    @param channel which channel of a possibly wide wave track
    */
   WaveChannelView(const std::shared_ptr<Track> &pTrack, size_t channel);
   ~WaveChannelView() override;

   // Preserve some view state too for undo/redo purposes
   void CopyTo( Track &track ) const override;

   std::shared_ptr<ChannelVRulerControls> DoGetVRulerControls() override;

   // CommonChannelView implementation
   void Reparent( const std::shared_ptr<Track> &parent ) override;

   static std::pair<
      bool, // if true, hit-testing is finished
      std::vector<UIHandlePtr>
   > DoDetailedHitTest(
      const TrackPanelMouseState &state,
      const AudacityProject *pProject, int currentTool, bool bMultiTool,
      const std::shared_ptr<WaveTrack> &wt,
      CommonChannelView &view);

   std::vector<WaveChannelSubView::Type> GetDisplays() const;
   void SetDisplay(Display display, bool exclusive = true);

   const WaveChannelSubViewPlacements &SavePlacements() const
      { return DoGetPlacements(); }
   void RestorePlacements(const WaveChannelSubViewPlacements &placements)
      { DoGetPlacements() = placements; }

   // Return true if successful.  Fails if you try to toggle off the only
   // sub-view.
   bool ToggleSubView( Display id );

   // Get all the sub-views, in a sequence that is unspecified but in
   // correspondence with the result of SavePlacements
   std::vector<std::shared_ptr<WaveChannelSubView>> GetAllSubViews();

   // Return cached height of rect in last call of GetSubViews
   wxCoord GetLastHeight() const { return mLastHeight; }

   bool GetMultiView() const { return DoGetMultiView(); }
   void SetMultiView( bool value ) { DoGetMultiView() = value; }


   std::weak_ptr<WaveClip> GetSelectedClip();

   // Returns a visible subset of subviews, sorted in the same 
   // order as they are supposed to be displayed
   

   // Get the visible sub-views,
   // if rect is provided then result will contain
   // y coordinate for each subview within this rect
   Refinement GetSubViews(const wxRect* rect = nullptr);

   unsigned CaptureKey
   (wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent,
       AudacityProject* project) override;

   unsigned KeyDown(wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent,
       AudacityProject* project) override;

   unsigned Char
   (wxKeyEvent& event, ViewInfo& viewInfo, wxWindow* pParent,
       AudacityProject* project) override;

   unsigned LoseFocus(AudacityProject *project) override;

   static bool ClipDetailsVisible(const WaveClip& clip, const ZoomInfo& zoomInfo, const wxRect& viewRect);
   static wxRect ClipHitTestArea(const WaveClip& clip, const ZoomInfo& zoomInfo, const wxRect& viewRect);
   static bool HitTest(const WaveClip& clip, const ZoomInfo& zoomInfo, const wxRect& rect, const wxPoint& pos);

   //FIXME: These functions do not push state to undo history
   //because attempt to do so leads to a focus lose which, in
   //turn finalizes text editing (state is saved after text
   //editing was intentionally finished instead)

   bool CutSelectedText(AudacityProject& project);
   bool CopySelectedText(AudacityProject& project);
   bool PasteText(AudacityProject& project);
   bool SelectAllText(AudacityProject& project);

private:
   void BuildSubViews() const;
   void DoSetDisplay(Display display, bool exclusive = true);
   bool SelectNextClip(ViewInfo& viewInfo, AudacityProject* project, bool forward);

   // TrackPanelDrawable implementation
   void Draw(
      TrackPanelDrawingContext &context,
      const wxRect &rect, unsigned iPass ) override;

   std::vector<UIHandlePtr> DetailedHitTest
      (const TrackPanelMouseState &state,
       const AudacityProject *pProject, int currentTool, bool bMultiTool)
      override;

   // ChannelView implementation
   Refinement GetSubViews(const wxRect& rect) override;

private:
   std::shared_ptr<CommonTrackCell> GetAffordanceControls() override;

   void DoSetMinimized( bool minimized ) override;

   // Placements are in correspondence with the array of sub-views
   // in the WaveChannelSubViews base class, though their sequence is
   // unspecified and maybe different in different platforms.
   WaveChannelSubViewPlacements &DoGetPlacements();
   const WaveChannelSubViewPlacements &DoGetPlacements() const;
   mutable wxCoord mLastHeight{};

   bool &DoGetMultiView();
   bool DoGetMultiView() const;

   std::shared_ptr<CommonTrackCell> DoGetAffordance(const std::shared_ptr<Track>& track);

   std::shared_ptr<CommonTrackCell> mpAffordanceCellControl;

   std::weak_ptr<TrackPanelCell> mKeyEventDelegate;

   std::weak_ptr<WaveTrackAffordanceHandle> mAffordanceHandle;
};

// Helper for drawing routines
class SelectedRegion;
class WaveClip;
class ZoomInfo;

struct AUDACITY_DLL_API ClipParameters
{
   // Do a bunch of calculations common to waveform and spectrum drawing.
   ClipParameters(
      bool spectrum, const SampleTrack *track,
      const WaveClip *clip, const wxRect &rect,
      const SelectedRegion &selectedRegion, const ZoomInfo &zoomInfo);

   double tOffset;
   double rate;
   double h; // absolute time of left edge of display
   double tpre; // offset corrected time of left edge of display
   double h1;
   double tpost; // offset corrected time of right edge of display

   // Calculate actual selection bounds so that t0 > 0 and t1 < the
   // end of the track
   double t0;
   double t1;

   double averagePixelsPerSample;
   bool showIndividualSamples;

   sampleCount ssel0;
   sampleCount ssel1;

   wxRect hiddenMid;
   int hiddenLeftOffset;

   wxRect mid;
   int leftOffset;

   // returns a clip rectangle restricted by viewRect, 
   // and with clipOffsetX - clip horizontal origin offset within view rect
   static wxRect GetClipRect(const WaveClip& clip, const ZoomInfo& zoomInfo, const wxRect& viewRect, bool* outShowSamples = nullptr);
};

#endif
