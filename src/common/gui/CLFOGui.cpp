//-------------------------------------------------------------------------------------------------------
//	Copyright 2005 Claes Johanson & Vember Audio
//-------------------------------------------------------------------------------------------------------
#include "CLFOGui.h"
#include "LfoModulationSource.h"

using namespace VSTGUI;
using namespace std;

extern CFontRef displayFont;
extern CFontRef patchNameFont;
extern CFontRef lfoTypeFont;

void drawtri(CRect r, CDrawContext* context, int orientation)
{
   int m = 2;
   int startx = r.left + m + 1;
   int endx = r.right - m;
   int midy = (r.top + r.bottom) * 0.5;
   int a = 0;
   if (orientation > 0)
      a = (endx - startx) - 1;

   for (int x = startx; x < endx; x++)
   {
      for (int y = (midy - a); y <= (midy + a); y++)
      {
         context->drawPoint(CPoint(x, y), kWhiteCColor);
      }
      a -= orientation;
      a = max(a, 0);
   }
}

void CLFOGui::draw(CDrawContext* dc)
{
   assert(lfodata);
   assert(storage);
   assert(ss);

   auto size = getViewSize();
   CRect outer(size);
   outer.inset(margin, margin);
   CRect leftpanel(outer);
   CRect maindisp(outer);
   leftpanel.right = lpsize + leftpanel.left;
   maindisp.left = leftpanel.right + 4 + 15;
   maindisp.top += 1;
   maindisp.bottom -= 1;

   if (ss && lfodata->shape.val.i == ls_stepseq)
   {
       cdisurf->begin();
#if MAC
       cdisurf->clear(0x0090ffff);
#else
       cdisurf->clear(0xffff9000);
#endif
       int w = cdisurf->getWidth();
       int h = cdisurf->getHeight();

       // I know I could do the math to convert these colors but I would rather leave them as literals for the compiler
       // so we don't have to shift them at runtime. See issue #141 in surge github
#if MAC
#define PIX_COL( a, b ) b
#else
#define PIX_COL( a, b ) a
#endif
       // Step Sequencer Colors. Remember mac is 0xRRGGBBAA and mac is 0xAABBGGRR
       int stepMarker = PIX_COL( 0xff000000, 0x000000ff);
       int loopRegionHi = PIX_COL( 0xffc6e9c4, 0xc4e9c6ff);
       int loopRegionLo = PIX_COL( 0xffb6d9b4, 0xb4d9b6ff );
       int noLoopHi = PIX_COL( 0xffdfdfdf, 0xdfdfdfff );
       int noLoopLo = PIX_COL( 0xffcfcfcf, 0xcfcfcfff );
       int grabMarker = PIX_COL( 0x00087f00, 0x007f08ff ); // Surely you can't mean this to be fully transparent?
       // But leave non-mac unch
       
      for (int i = 0; i < n_stepseqsteps; i++)
      {
         CRect rstep(maindisp), gstep;
         rstep.offset(-size.left - splitpoint, -size.top);
         rstep.left += scale * i;
         rstep.right = rstep.left + scale - 1;
         rstep.bottom -= margin2 + 1;
         CRect shadow(rstep);
         shadow.inset(-1, -1);
         cdisurf->fillRect(shadow, skugga);
         if (edit_trigmask)
         {
            gstep = rstep;
            rstep.top += margin2;
            gstep.bottom = rstep.top - 1;
            gaterect[i] = gstep;
            gaterect[i].offset(size.left + splitpoint, size.top);

            if (ss->trigmask & (1 << i))
               cdisurf->fillRect(gstep, stepMarker);
            else if ((i >= ss->loop_start) && (i <= ss->loop_end))
               cdisurf->fillRect(gstep, (i & 3) ? loopRegionHi : loopRegionLo);
            else
               cdisurf->fillRect(gstep, (i & 3) ? noLoopHi : noLoopLo);
         }
         if ((i >= ss->loop_start) && (i <= ss->loop_end))
            cdisurf->fillRect(rstep, (i & 3) ? loopRegionHi : loopRegionLo);
         else
            cdisurf->fillRect(rstep, (i & 3) ? noLoopHi : noLoopLo);
         steprect[i] = rstep;
         steprect[i].offset(size.left + splitpoint, size.top);
         CRect v(rstep);
         int p1, p2;
         if (lfodata->unipolar.val.b)
         {
            v.top = v.bottom - (int)(v.getHeight() * ss->steps[i]);
         }
         else
         {
            p1 = v.bottom - (int)((float)0.5f + v.getHeight() * (0.5f + 0.5f * ss->steps[i]));
            p2 = (v.bottom + v.top) * 0.5;
            v.top = min(p1, p2);
            v.bottom = max(p1, p2) + 1;
         }
         // if (p1 == p2) p2++;
         cdisurf->fillRect(v, stepMarker);
      }

      rect_steps = steprect[0];
      rect_steps.right = steprect[n_stepseqsteps - 1].right;
      rect_steps_retrig = gaterect[0];
      rect_steps_retrig.right = gaterect[n_stepseqsteps - 1].right;

      rect_ls = maindisp;
      rect_ls.offset(-size.left - splitpoint, -size.top);
      rect_ls.top = rect_ls.bottom - margin2;
      rect_le = rect_ls;

      rect_ls.left += scale * ss->loop_start - 1;
      rect_ls.right = rect_ls.left + margin2;
      rect_le.right = rect_le.left + scale * (ss->loop_end + 1);
      rect_le.left = rect_le.right - margin2;

      cdisurf->fillRect(rect_ls, grabMarker);
      cdisurf->fillRect(rect_le, grabMarker);
      CRect linerect(rect_ls);
      linerect.top = maindisp.top - size.top;
      linerect.right = linerect.left + 1;
      cdisurf->fillRect(linerect, grabMarker);
      linerect = rect_le;
      linerect.top = maindisp.top - size.top;
      linerect.left = linerect.right - 1;
      cdisurf->fillRect(linerect, grabMarker);

      rect_ls.offset(size.left + splitpoint, size.top);
      rect_le.offset(size.left + splitpoint, size.top);

      
      CPoint sp(0, 0);
      CRect sr(size.left + splitpoint, size.top, size.right, size.bottom);
      cdisurf->commit();
      cdisurf->draw(dc, sr, sp);
   }
   else
   {
      CGraphicsPath *path = dc->createGraphicsPath();
      CGraphicsPath *eupath = dc->createGraphicsPath();
      CGraphicsPath *edpath = dc->createGraphicsPath();

      pdata tp[n_scene_params];
      {
         tp[lfodata->delay.param_id_in_scene].i = lfodata->delay.val.i;
         tp[lfodata->attack.param_id_in_scene].i = lfodata->attack.val.i;
         tp[lfodata->hold.param_id_in_scene].i = lfodata->hold.val.i;
         tp[lfodata->decay.param_id_in_scene].i = lfodata->decay.val.i;
         tp[lfodata->sustain.param_id_in_scene].i = lfodata->sustain.val.i;
         tp[lfodata->release.param_id_in_scene].i = lfodata->release.val.i;

         tp[lfodata->magnitude.param_id_in_scene].i = lfodata->magnitude.val.i;
         tp[lfodata->rate.param_id_in_scene].i = lfodata->rate.val.i;
         tp[lfodata->shape.param_id_in_scene].i = lfodata->shape.val.i;
         tp[lfodata->start_phase.param_id_in_scene].i = lfodata->start_phase.val.i;
         tp[lfodata->deform.param_id_in_scene].i = lfodata->deform.val.i;
         tp[lfodata->trigmode.param_id_in_scene].i = lm_keytrigger;
      }

      LfoModulationSource* tlfo = new LfoModulationSource();
      tlfo->assign(storage, lfodata, tp, 0, ss, true);
      tlfo->attack();
      CRect boxo(maindisp);
      boxo.offset(-size.left - splitpoint, -size.top);
      
      int totalSamples = ( 1 << 3 ) * (int)( boxo.right - boxo.left );
      int averagingWindow = 1;
      float valScale = 100.0;
      
      for (int i=0; i<totalSamples; i += averagingWindow )
      {
         float val = 0;
         float eval = 0;
         for (int s = 0; s < averagingWindow; s++)
         {
            tlfo->process_block();

            val  += tlfo->output;
            eval += tlfo->env_val * lfodata->magnitude.val.f;
         }
         val = val / averagingWindow;
         eval = eval / averagingWindow;
         val  = ( ( - val + 1.0f ) * 0.5 * 0.8 + 0.1 ) * valScale;
         float euval = ( ( - eval + 1.0f ) * 0.5 * 0.8 + 0.1 ) * valScale;
         float edval = ( ( eval + 1.0f ) * 0.5 * 0.8 + 0.1 ) * valScale;
      
         float xc = valScale * i / totalSamples;

         if( i == 0 )
         {
             path->beginSubpath(xc, val );
             eupath->beginSubpath(xc, euval);
             edpath->beginSubpath(xc, edval);
         }
         else
         {
             path->addLine(xc, val );
             eupath->addLine(xc, euval);
             edpath->addLine(xc, edval);
         }
      }
      delete tlfo;

      VSTGUI::CGraphicsTransform tf = VSTGUI::CGraphicsTransform()
          .scale(boxo.getWidth()/valScale, boxo.getHeight() / valScale )
          .translate(boxo.getTopLeft().x, boxo.getTopLeft().y )
          .translate(maindisp.getTopLeft().x, maindisp.getTopLeft().y );

      dc->saveGlobalState();

      // OK so draw the rules
      CPoint mid0(0, valScale/2.f), mid1(valScale,valScale/2.f);
      CPoint top0(0, valScale * 0.9), top1(valScale,valScale * 0.9);
      CPoint bot0(0, valScale * 0.1), bot1(valScale,valScale * 0.1);
      tf.transform(mid0);
      tf.transform(mid1);
      tf.transform(top0);
      tf.transform(top1);
      tf.transform(bot0);
      tf.transform(bot1);
      dc->setDrawMode(VSTGUI::kAntiAliasing);

      dc->setLineWidth(1.0);
      dc->setFrameColor(VSTGUI::CColor(0xE0, 0x80, 0x00));
      dc->drawLine(mid0, mid1);
      
      dc->setLineWidth(1.0);
      dc->setFrameColor(VSTGUI::CColor(0xE0, 0x80, 0x00));
      dc->drawLine(top0, top1);
      dc->drawLine(bot0, bot1);
      
      dc->setLineWidth(1.3);
      dc->setFrameColor(VSTGUI::CColor(0x00, 0x00, 0, 0xFF));
      dc->drawGraphicsPath(path, VSTGUI::CDrawContext::PathDrawMode::kPathStroked, &tf );

      dc->setLineWidth(1.0);
      dc->setFrameColor(VSTGUI::CColor(0xB0, 0x60, 0x00, 0xFF));
      dc->drawGraphicsPath(eupath, VSTGUI::CDrawContext::PathDrawMode::kPathStroked, &tf );
      dc->drawGraphicsPath(edpath, VSTGUI::CDrawContext::PathDrawMode::kPathStroked, &tf );
      dc->restoreGlobalState();
      path->forget();
      eupath->forget();
      edpath->forget();
   }

   CColor cskugga = {0x5d, 0x5d, 0x5d, 0xff};
   CColor cgray = {0x97, 0x98, 0x9a, 0xff};
   CColor cselected = {0xfe, 0x98, 0x15, 0xff};
   // CColor blackColor (0, 0, 0, 0);
   dc->setFrameColor(cskugga);
   dc->setFont(lfoTypeFont);

   rect_shapes = leftpanel;
   for (int i = 0; i < n_lfoshapes; i++)
   {
      CRect tb(leftpanel);
      tb.top = leftpanel.top + 10 * i;
      tb.bottom = tb.top + 10;
      if (i == lfodata->shape.val.i)
      {
         CRect tb2(tb);
         tb2.left++;
         tb2.top += 0.5;
         tb2.inset(2, 1);
         tb2.offset(0, 1);
         dc->setFillColor(cselected);
         dc->drawRect(tb2, kDrawFilled);
      }
      // else dc->setFillColor(cgray);
      // dc->fillRect(tb);
      shaperect[i] = tb;
      // tb.offset(0,-1);
      dc->setFontColor(kBlackCColor);
      tb.top += 1.6; // now the font is smaller and the box is square, smidge down the text
      dc->drawString(ls_abberations[i], tb);
   }

   if (ss && lfodata->shape.val.i == ls_stepseq)
   {
      ss_shift_left = leftpanel;
      ss_shift_left.offset(53, 23);
      ss_shift_left.right = ss_shift_left.left + 12;
      ss_shift_left.bottom = ss_shift_left.top + 34;

      dc->setFillColor(cskugga);
      dc->drawRect(ss_shift_left, kDrawFilled);
      ss_shift_left.inset(1, 1);
      ss_shift_left.bottom = ss_shift_left.top + 16;

      dc->setFillColor(cgray);
      dc->drawRect(ss_shift_left, kDrawFilled);
      drawtri(ss_shift_left, dc, -1);

      ss_shift_right = ss_shift_left;
      ss_shift_right.offset(0, 16);
      dc->setFillColor(cgray);
      dc->drawRect(ss_shift_right, kDrawFilled);
      drawtri(ss_shift_right, dc, 1);
      // ss_shift_left,ss_shift_right;
   }

   setDirty(false);
}

enum
{
   cs_null = 0,
   cs_shape,
   cs_steps,
   cs_trigtray_false,
   cs_trigtray_true,
   cs_loopstart,
   cs_loopend,

};

CMouseEventResult CLFOGui::onMouseDown(CPoint& where, const CButtonState& buttons)
{
   if (1) //(buttons & kLButton))
   {
      if (ss && lfodata->shape.val.i == ls_stepseq)
      {
         if (rect_steps.pointInside(where))
         {
            controlstate = cs_steps;
            onMouseMoved(where, buttons);
            return kMouseEventHandled;
         }
         else if (rect_steps_retrig.pointInside(where))
         {
            bool gatestate = false;
            for (int i = 0; i < (n_stepseqsteps); i++)
            {
               if (gaterect[i].pointInside(where))
               {
                  gatestate = ss->trigmask & (1 << i);
               }
            }
            controlstate = gatestate ? cs_trigtray_true : cs_trigtray_false;
            onMouseMoved(where, buttons);
            return kMouseEventHandled;
         }
         else if (ss_shift_left.pointInside(where))
         {
            float t = ss->steps[0];
            for (int i = 0; i < (n_stepseqsteps - 1); i++)
            {
               ss->steps[i] = ss->steps[i + 1];
               assert((i >= 0) && (i < n_stepseqsteps));
            }
            ss->steps[n_stepseqsteps - 1] = t;
            ss->trigmask = ((ss->trigmask & 0xfffe) >> 1) | ((ss->trigmask & 1) << 15);
            invalid();
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
         }
         else if (ss_shift_right.pointInside(where))
         {
            float t = ss->steps[n_stepseqsteps - 1];
            for (int i = (n_stepseqsteps - 2); i >= 0; i--)
            {
               ss->steps[i + 1] = ss->steps[i];
               assert((i >= 0) && (i < n_stepseqsteps));
            }
            ss->steps[0] = t;
            ss->trigmask = ((ss->trigmask & 0x7fff) << 1) | ((ss->trigmask & 0x8000) >> 15);
            invalid();
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
         }
      }
      if (rect_ls.pointInside(where))
      {
         controlstate = cs_loopstart;
         return kMouseEventHandled;
      }
      else if (rect_le.pointInside(where))
      {
         controlstate = cs_loopend;
         return kMouseEventHandled;
      }
      else if (rect_shapes.pointInside(where))
      {
         controlstate = cs_shape;
         onMouseMoved(where, buttons);
         return kMouseEventHandled;
      }
   }
   return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}
CMouseEventResult CLFOGui::onMouseUp(CPoint& where, const CButtonState& buttons)
{
   if (controlstate)
   {
      // onMouseMoved(where,buttons);
      controlstate = cs_null;
   }
   return kMouseEventHandled;
}

CMouseEventResult CLFOGui::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
   if (controlstate == cs_shape)
   {
      for (int i = 0; i < n_lfoshapes; i++)
      {
         if (shaperect[i].pointInside(where))
         {
            if (lfodata->shape.val.i != i)
            {
               lfodata->shape.val.i = i;
               invalid();
            }
         }
      }
   }
   else if (controlstate == cs_loopstart)
   {
      ss->loop_start = limit_range((int)(where.x - steprect[0].left + (scale >> 1)) / scale, 0,
                                   n_stepseqsteps - 1);
      invalid();
   }
   else if (controlstate == cs_loopend)
   {
      ss->loop_end = limit_range((int)(where.x - steprect[0].left - (scale >> 1)) / scale, 0,
                                 n_stepseqsteps - 1);
      invalid();
   }
   else if (controlstate == cs_steps)
   {
      for (int i = 0; i < n_stepseqsteps; i++)
      {
         if ((where.x > steprect[i].left) && (where.x < steprect[i].right))
         {
            float f = (float)(steprect[i].bottom - where.y) / steprect[i].getHeight();
            if (buttons & (kControl | kRButton))
               f = 0;
            else if (lfodata->unipolar.val.b)
               f = limit_range(f, 0.f, 1.f);
            else
               f = limit_range(f * 2.f - 1.f, -1.f, 1.f);
            if (buttons & kShift)
            {
               f *= 12;
               f = floor(f);
               f *= (1.f / 12.f);
            }
            ss->steps[i] = f;
            invalid();
         }
      }
   }
   else if ((controlstate == cs_trigtray_false) || (controlstate == cs_trigtray_true))
   {
      for (int i = 0; i < n_stepseqsteps; i++)
      {
         if ((where.x > gaterect[i].left) && (where.x < gaterect[i].right))
         {
            unsigned int m = 1 << i;
            unsigned int minv = m ^ 0xffffffff;
            ss->trigmask =
                (ss->trigmask & minv) |
                (((controlstate == cs_trigtray_true) || (buttons & (kControl | kRButton))) ? 0 : m);
            invalid();
         }
      }
   }
   return kMouseEventHandled;
}
