#include "frame_scheduler.h"
#include "shared_vars.h"
#include "ruby_central.h"
#include "render_commands.h"
#include "renderer/render_engine.h"

static FrameClock clockState;

const FrameClock& ui_frame_clock() { return clockState; }

void ui_present_frame(const std::function<void()>& draw)
{
   if (!g_pRenderEngine || g_pRenderEngine->isFrameStarted()) return;
   clockState.begin(get_current_timestamp_ms());
   auto start = get_current_timestamp_micros();
   g_pRenderEngine->startFrame();
   draw();
   auto beforeSubmit = get_current_timestamp_micros();
   bool displayed = g_pRenderEngine->endFrame();
   auto afterSubmit = get_current_timestamp_micros();
   auto now = get_current_timestamp_ms();
   clockState.finish(now, displayed);
   static uint32_t lastReport = 0;
   if (g_pControllerSettings && g_pControllerSettings->iDeveloperMode && now - lastReport >= 5000) {
      log_line("[UI] %u FPS; interval p50/p95 %u/%u ms; latest draw/submit %u/%u us; total failed %u",
               clockState.fps, clockState.p50, clockState.p95,
               (unsigned)(beforeSubmit - start), (unsigned)(afterSubmit - beforeSubmit), clockState.failed);
      lastReport = now;
   }
}

void ui_present_progress(ProgressOverlay overlay)
{
   ui_present_frame([overlay] {
      render_background_and_paddings(false);
      popups_render();
      if (overlay == ProgressOverlay::Commands) render_commands();
      popups_render_topmost();
   });
}

void ui_clear_frame() { ui_present_frame([] {}); }
