#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <SFML/Graphics.hpp>

#define RAD2DEG(x) x*(180.0/M_PI)
#define DEG2RAD(x) x/(180.0/M_PI)

#define DERIV_SCREEN_IX 50
#define DERIV_SCREEN_IY 50
#define INTEG_SCREEN_IX 710
#define INTEG_SCREEN_IY 50

typedef struct
{
      float x,y;
} Vector2f;

typedef struct
{
      Vector2f gdims[2];
      Vector2f windims[2];
      Vector2f gipos[2];
      int tickerDensity;
} wininfo;

bool isOnGraph(const wininfo &inf, const Vector2f &gcoords, int const &gnum)
{
      if(gcoords.x < (inf.gipos[gnum].x+inf.gdims[gnum].x) && 
            gcoords.x > (inf.gipos[gnum].x) && 
            -gcoords.y > (inf.gipos[gnum].y-inf.gdims[gnum].y) && 
            -gcoords.y < (inf.gipos[gnum].y))
      {
            return true;
      }
      else
      {
            return false;
      }
}

void MapWindowCoords(const wininfo &inf, Vector2f &wcoords, const Vector2f &gcoords, int const &map_which)
{
      if(map_which > 1 || map_which < 0)
      {
            return;
      }

      Vector2f tpos;
      tpos.x = gcoords.x - inf.gipos[map_which].x;
      tpos.y = gcoords.y + inf.gipos[map_which].y;
      tpos.x *= inf.windims[map_which].x/inf.gdims[map_which].x;
      tpos.y *= inf.windims[map_which].y/inf.gdims[map_which].y;
      wcoords.x = tpos.x;
      wcoords.y = tpos.y;
}

void MapGraphCoords(const wininfo &inf, Vector2f &gcoords, const Vector2f &wcoords, int const &map_which)
{
      if(map_which > 1 || map_which < 0)
      {
            return;
      }

      Vector2f tpos;
      tpos.x = wcoords.x * (inf.gdims[map_which].x/inf.windims[map_which].x);
      tpos.y = wcoords.y * (inf.gdims[map_which].y/inf.windims[map_which].y);
      tpos.x += inf.gipos[map_which].x;
      tpos.y -= inf.gipos[map_which].y;
      gcoords.x = tpos.x;
      gcoords.y = tpos.y;
}

bool load_options(wininfo &inf, float &iny)
{
      FILE * optfile = fopen("options.cfg", "r");
      if(!optfile)
      {
            return false;
      }

      char fline[256];

      do
      {
            fgets(fline, sizeof(fline), optfile);
      } while(fline[0] == ';');
      if(feof(optfile))
      {
            return false;
      }
      if(sscanf(fline, "%d", &inf.tickerDensity) < 1)
      {
            return false;
      }

      do
      {
            fgets(fline, sizeof(fline), optfile);
      } while(fline[0] == ';');
      if(feof(optfile))
      {
            return false;
      }
      if(sscanf(fline, "(%f, %f)", &inf.gipos[0].x, &inf.gipos[0].y) < 2)
      {
            return false;
      }

      do
      {
            fgets(fline, sizeof(fline), optfile);
      } while(fline[0] == ';');
      if(feof(optfile))
      {
            return false;
      }
      if(sscanf(fline, "(%f, %f)", &inf.gipos[1].x, &inf.gipos[1].y) < 2)
      {
            return false;
      }

      do
      {
            fgets(fline, sizeof(fline), optfile);
      } while(fline[0] == ';');
      if(feof(optfile))
      {
            return false;
      }
      if(sscanf(fline, "%f : %f", &inf.gdims[0].x, &inf.gdims[0].y) < 2)
      {
            return false;
      }

      do
      {
            fgets(fline, sizeof(fline), optfile);
      } while(fline[0] == ';');
      if(feof(optfile))
      {
            return false;
      }
      if(sscanf(fline, "%f : %f", &inf.gdims[1].x, &inf.gdims[1].y) < 2)
      {
            return false;
      }

      fclose(optfile);

      iny *= -1.0f;
      return true;
}

int main(int argc, char **argv)
{
      wininfo winInfo;
      winInfo.gdims[0].x = 40.0f;
      winInfo.gdims[0].y = 40.0f;
      winInfo.gdims[1].x = 40.0f;
      winInfo.gdims[1].y = 40.0f;
      winInfo.windims[0].x = 500.0f;
      winInfo.windims[0].y = 500.0f;
      winInfo.windims[1].x = 500.0f;
      winInfo.windims[1].y = 500.0f;
      winInfo.tickerDensity = 4;

      winInfo.gipos[0].x = -20.0f;
      winInfo.gipos[0].y = 20.0f;
      winInfo.gipos[1].x = -20.0f;
      winInfo.gipos[1].y = 20.0f;

      float scale=1.00f;
      float iny=0.0f;
      if(argc > 1)
      {
            iny = atof(argv[1]);
      }
      if(load_options(winInfo, iny))
      {
            printf("Upper-left point of derivative graph: (%f, %f)\n", winInfo.gipos[0].x, winInfo.gipos[0].y);
            printf("Upper-left point of function graph: (%f, %f)\n", winInfo.gipos[1].x, winInfo.gipos[1].y);
            printf("Derivative graph dimensions: %f x %f\n", winInfo.gdims[0].x, winInfo.gdims[0].y);
            printf("Function graph dimensions: %f x %f\n", winInfo.gdims[1].x, winInfo.gdims[1].y);
            printf("Using C=%f\n", -iny);
      }
      else
      {
            printf("Error opening options file\n");
      }
	bool bFiring = false,bFireReset = false;
      bool bEnableTickers=true;
	char ttext[256];
	sf::Clock clock;
	sf::RenderWindow sWindow(sf::VideoMode(1280, 720, 8), argv[0]);
      sWindow.UseVerticalSync(true);
	sf::Shape projectile = sf::Shape::Circle(0.0f, 0.0f, 5.0f, sf::Color(255.0f, 64.0f, 64.0f));
      sf::Shape win_curv = sf::Shape::Rectangle(DERIV_SCREEN_IX, DERIV_SCREEN_IY, winInfo.windims[0].x+DERIV_SCREEN_IX, winInfo.windims[0].y+DERIV_SCREEN_IY, sf::Color(0.0f, 0.0f, 0.0f), 2.0f, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape win_int = sf::Shape::Rectangle(INTEG_SCREEN_IX, INTEG_SCREEN_IY, winInfo.windims[1].x+INTEG_SCREEN_IX, winInfo.windims[1].y+INTEG_SCREEN_IY, sf::Color(0.0f, 0.0f, 0.0f), 2.0f, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape curv_xax =  sf::Shape::Line(0.0f, 0.0f, winInfo.windims[0].x, 0.0f, 1, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape int_xax =  sf::Shape::Line(0.0f, 0.0f, winInfo.windims[1].x, 0.0f, 1, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape curv_yax =  sf::Shape::Line(0.0f, 0.0f, 0.0f, winInfo.windims[0].y, 1, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape int_yax =  sf::Shape::Line(0.0f, 0.0f, 0.0f, winInfo.windims[1].y, 1, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape ticklinex = sf::Shape::Line(0.0f, 5.0f, 0.0f, -5.0f, 1, sf::Color(255.0f, 255.0f, 255.0f));
      sf::Shape tickliney = sf::Shape::Line(5.0f, 0.0f, -5.0f, 0.0f, 1, sf::Color(255.0f, 255.0f, 255.0f));
      sf::String sDerivText("f'(x)", sf::Font::GetDefaultFont(), 24);
      sf::String sFuncText("f(x)", sf::Font::GetDefaultFont(), 24);
      sf::String sTrackText("Tracking position: (0, 0)", sf::Font::GetDefaultFont(), 18);
      sf::String sTickText("0", sf::Font::GetDefaultFont(), 16);
      sf::Shape tline;

      bool bDrawing=false;
      bool bDrawProjectile=false;
      int ct=0,k=0,j=0;
      float cx=0.0f,cy=0.0f;
	Vector2f deriv[4096];
      Vector2f integ[4096];

      Vector2f zvec;
      zvec.x = 0.0f;
      zvec.y = 0.0f;

      memset(deriv, 0, sizeof(Vector2f)*4096);
      memset(integ, 0, sizeof(Vector2f)*4096);

      sDerivText.SetStyle(sf::String::Italic);
      sFuncText.SetStyle(sf::String::Italic);
      sDerivText.SetPosition(DERIV_SCREEN_IX+(0.5f*winInfo.windims[0].x)-20.0f, DERIV_SCREEN_IY+winInfo.windims[0].y+40.0f);
      sFuncText.SetPosition(INTEG_SCREEN_IX+(0.5f*winInfo.windims[1].x)-20.0f, INTEG_SCREEN_IY+winInfo.windims[1].y+40.0f);

	while(sWindow.IsOpened())
	{
		sf::Event sEvent;
		while(sWindow.GetEvent(sEvent))
		{
			switch(sEvent.Type)
			{
			case sf::Event::MouseButtonPressed:
                        if(sEvent.MouseButton.Button == sf::Mouse::Left)
                        {
                              bDrawing = true;
                        }
                        else if(sEvent.MouseButton.Button == sf::Mouse::Right)
                        {
                              bDrawProjectile = true;
                        }
				break;
                  case sf::Event::MouseButtonReleased:
                        if(sEvent.MouseButton.Button == sf::Mouse::Left)
                        {
                              bDrawing = false;
                        }
                        else if(sEvent.MouseButton.Button == sf::Mouse::Right)
                        {
                              bDrawProjectile = false;
                        }
				break;
                  case sf::Event::KeyPressed:
                        memset(deriv, 0, sizeof(Vector2f)*4096);
                        memset(integ, 0, sizeof(Vector2f)*4096);
                        ct = 0;
                        bDrawing=false;
				break;
			}
		}

            if(bDrawing)
            {
                  float tlf = iny,dx=0.0f;
                  Vector2f gco;
                  gco.x = sWindow.GetInput().GetMouseX()-DERIV_SCREEN_IX;
                  gco.y = sWindow.GetInput().GetMouseY()-DERIV_SCREEN_IY;
                  MapGraphCoords(winInfo, deriv[ct], gco, 0);
                  if(isOnGraph(winInfo, deriv[ct], 0))
                  {
                        if(ct == 0)
                        {
                              winInfo.gipos[0].x = (-deriv[ct].x)+(winInfo.gipos[0].x);
                              winInfo.gipos[1].x = (-deriv[ct].x)+(winInfo.gipos[1].x);
                              deriv[ct].x = 0.0f;
                        }
                        ct++;
                        for(j=0;j<(ct-1);j++)
                        {
                              dx = deriv[j+1].x-deriv[j].x;
                              tlf += 0.5f*(deriv[j+1].y-deriv[j].y)*dx + deriv[j].y*dx;
                        }
                        integ[ct-1].x = deriv[ct-1].x;
                        integ[ct-1].y = scale*tlf;
                  }
            }

		sWindow.Clear();

		sWindow.Draw(win_curv);
            sWindow.Draw(win_int);

            {
                  Vector2f tpos;
                  MapWindowCoords(winInfo, tpos, zvec, 0);
                  if(tpos.y > 0.0f && tpos.y < winInfo.windims[0].y)
                  {
                        curv_xax.SetPosition(sf::Vector2f(DERIV_SCREEN_IX, tpos.y+DERIV_SCREEN_IY));
                        sWindow.Draw(curv_xax);
                  }
                  if(tpos.x > 0.0f && tpos.x < winInfo.windims[0].x)
                  {
                        curv_yax.SetPosition(sf::Vector2f(tpos.x+DERIV_SCREEN_IX, DERIV_SCREEN_IY));
                        sWindow.Draw(curv_yax);
                  }
                  MapWindowCoords(winInfo, tpos, zvec, 1);
                  if(tpos.y > 0.0f && tpos.y < winInfo.windims[1].y)
                  {
                        int_xax.SetPosition(sf::Vector2f(INTEG_SCREEN_IX, tpos.y+INTEG_SCREEN_IY));
                        sWindow.Draw(int_xax);
                  }
                  if(tpos.x > 0.0f && tpos.x < winInfo.windims[1].x)
                  {
                        int_yax.SetPosition(sf::Vector2f(tpos.x+INTEG_SCREEN_IX, INTEG_SCREEN_IY));
                        sWindow.Draw(int_yax);
                  }
            }

            if(bDrawProjectile)
            {
                  char tracktext[256];
                  bool bRenderTickline=false;
                  Vector2f spos,gpos,ipos,npos,t1pos,t2pos;
                  spos.x = sWindow.GetInput().GetMouseX();
                  spos.y = sWindow.GetInput().GetMouseY();
                  if(spos.x > DERIV_SCREEN_IX && spos.x < DERIV_SCREEN_IX+winInfo.windims[0].x
                        && spos.y > DERIV_SCREEN_IY && spos.y < DERIV_SCREEN_IY+winInfo.windims[0].y)
                  {
                        spos.x -= DERIV_SCREEN_IX;
                        spos.y -= DERIV_SCREEN_IY;
                        MapGraphCoords(winInfo, gpos, spos, 0);
                        MapWindowCoords(winInfo, ipos, gpos, 1);
                        MapGraphCoords(winInfo, npos, ipos, 1);
                        for(k=1;k<ct;k++)
                        {
                              if(gpos.x >= deriv[k-1].x && gpos.x <= deriv[k].x)
                              {
                                    bRenderTickline=true;
                                    break;
                              }
                        }
                        if(bRenderTickline)
                        {
                              for(j=1;j<ct;j++)
                              {
                                    if(gpos.x >= integ[j-1].x && gpos.x <= integ[j].x)
                                    {
                                          break;
                                    }
                              }
                              Vector2f c1pos,c2pos;
                              c1pos.x = gpos.x;
                              c1pos.y = (deriv[k].y+deriv[k-1].y)/2.0f;
                              c2pos.x = gpos.x;
                              c2pos.y = (integ[j].y+integ[j-1].y)/2.0f;
                              MapWindowCoords(winInfo, t1pos, c1pos, 0);
                              MapWindowCoords(winInfo, t2pos, c2pos, 1);
                              ticklinex.SetPosition(sf::Vector2f(t1pos.x+DERIV_SCREEN_IX, t1pos.y+DERIV_SCREEN_IY));
                              sWindow.Draw(ticklinex);
                              ticklinex.SetPosition(sf::Vector2f(t2pos.x+INTEG_SCREEN_IX, t2pos.y+INTEG_SCREEN_IY));
                              sWindow.Draw(ticklinex);
                              sprintf(tracktext, "f'(%0.2f) = %0.2f", gpos.x, -(deriv[k].y+deriv[k-1].y)/2.0f);
                              sTrackText.SetText(tracktext);
                              sTrackText.SetPosition(20.0f, 660.0f);
                              sWindow.Draw(sTrackText);
                              sprintf(tracktext, "f(%0.2f) = %0.2f", gpos.x, -(integ[j].y+integ[j-1].y)/2.0f);
                              sTrackText.SetText(tracktext);
                              sTrackText.SetPosition(20.0f, 680.0f);
                              sWindow.Draw(sTrackText);
                        }
                        sprintf(tracktext, "Tracking: (%0.2f, %0.2f)", gpos.x, -gpos.y);
                        sTrackText.SetText(tracktext);
                        sTrackText.SetPosition(20.0f, 640.0f);
                        sWindow.Draw(sTrackText);
                        if(isOnGraph(winInfo, gpos, 0))
                        {
                              projectile.SetPosition(sf::Vector2f(spos.x+DERIV_SCREEN_IX, spos.y+DERIV_SCREEN_IY));
                              sWindow.Draw(projectile);
                        }
                        if(isOnGraph(winInfo, npos, 1))
                        {
                              projectile.SetPosition(sf::Vector2f(ipos.x+INTEG_SCREEN_IX, ipos.y+INTEG_SCREEN_IY));
                              sWindow.Draw(projectile);
                        }
                  }
                  else if(spos.x > INTEG_SCREEN_IX && spos.x < INTEG_SCREEN_IX+winInfo.windims[1].x
                        && spos.y > INTEG_SCREEN_IY && spos.y < INTEG_SCREEN_IY+winInfo.windims[1].y)
                  {
                        spos.x -= INTEG_SCREEN_IX;
                        spos.y -= INTEG_SCREEN_IY;
                        MapGraphCoords(winInfo, gpos, spos, 1);
                        MapWindowCoords(winInfo, ipos, gpos, 0);
                        MapGraphCoords(winInfo, npos, ipos, 0);
                        for(k=1;k<ct;k++)
                        {
                              if(gpos.x >= integ[k-1].x && gpos.x <= integ[k].x)
                              {
                                    bRenderTickline=true;
                                    break;
                              }
                        }
                        if(bRenderTickline)
                        {
                              for(j=1;j<ct;j++)
                              {
                                    if(gpos.x >= deriv[j-1].x && gpos.x <= deriv[j].x)
                                    {
                                          break;
                                    }
                              }
                              Vector2f c1pos,c2pos;
                              c1pos.x = gpos.x;
                              c1pos.y = (integ[k].y+integ[k-1].y)/2.0f;
                              c2pos.x = gpos.x;
                              c2pos.y = (deriv[j].y+deriv[j-1].y)/2.0f;
                              MapWindowCoords(winInfo, t1pos, c1pos, 1);
                              MapWindowCoords(winInfo, t2pos, c2pos, 0);
                              ticklinex.SetPosition(sf::Vector2f(t1pos.x+INTEG_SCREEN_IX, t1pos.y+INTEG_SCREEN_IY));
                              sWindow.Draw(ticklinex);
                              ticklinex.SetPosition(sf::Vector2f(t2pos.x+DERIV_SCREEN_IX, t2pos.y+DERIV_SCREEN_IY));
                              sWindow.Draw(ticklinex);
                              sprintf(tracktext, "f'(%0.2f) = %0.2f", gpos.x, -(deriv[j].y+deriv[j-1].y)/2.0f);
                              sTrackText.SetText(tracktext);
                              sTrackText.SetPosition(20.0f, 660.0f);
                              sWindow.Draw(sTrackText);
                              sprintf(tracktext, "f(%0.2f) = %0.2f", gpos.x, -(integ[k].y+integ[k-1].y)/2.0f);
                              sTrackText.SetText(tracktext);
                              sTrackText.SetPosition(20.0f, 680.0f);
                              sWindow.Draw(sTrackText);
                        }
                        sprintf(tracktext, "Tracking: (%0.2f, %0.2f)", gpos.x, -gpos.y);
                        sTrackText.SetText(tracktext);
                        sTrackText.SetPosition(20.0f, 640.0f);
                        sWindow.Draw(sTrackText);
                        if(isOnGraph(winInfo, npos, 0))
                        {
                              projectile.SetPosition(sf::Vector2f(ipos.x+DERIV_SCREEN_IX, ipos.y+DERIV_SCREEN_IY));
                              sWindow.Draw(projectile);
                        }
                        if(isOnGraph(winInfo, gpos, 1))
                        {
                              projectile.SetPosition(sf::Vector2f(spos.x+INTEG_SCREEN_IX, spos.y+INTEG_SCREEN_IY));
                              sWindow.Draw(projectile);
                        }
                  }
            }

            for(k=0;k<=winInfo.tickerDensity;k++)
            {
                  char tracktext[16];
                  Vector2f tpos,gpos;

                  tpos.x = k*(winInfo.windims[0].x/winInfo.tickerDensity);
                  tpos.y = winInfo.windims[0].y;
                  MapGraphCoords(winInfo, gpos, tpos, 0);
                  sprintf(tracktext, "%0.2f", gpos.x);
                  sTickText.SetText(tracktext);
                  sTickText.SetPosition(sf::Vector2f(tpos.x+DERIV_SCREEN_IX-16.0f, tpos.y+16.0f+DERIV_SCREEN_IY));
                  ticklinex.SetPosition(sf::Vector2f(tpos.x+DERIV_SCREEN_IX, tpos.y+DERIV_SCREEN_IY));
                  sWindow.Draw(sTickText);
                  sWindow.Draw(ticklinex);

                  tpos.x = winInfo.windims[0].x;
                  tpos.y = k*(winInfo.windims[0].y/winInfo.tickerDensity);
                  MapGraphCoords(winInfo, gpos, tpos, 0);
                  sprintf(tracktext, "%0.2f", -gpos.y);
                  sTickText.SetText(tracktext);
                  sTickText.SetPosition(sf::Vector2f(tpos.x+DERIV_SCREEN_IX+16.0f, tpos.y+DERIV_SCREEN_IY-8.0f));
                  tickliney.SetPosition(sf::Vector2f(tpos.x+DERIV_SCREEN_IX, tpos.y+DERIV_SCREEN_IY));
                  sWindow.Draw(sTickText);
                  sWindow.Draw(tickliney);

                  tpos.x = k*(winInfo.windims[1].x/winInfo.tickerDensity);
                  tpos.y = winInfo.windims[1].y;
                  MapGraphCoords(winInfo, gpos, tpos, 1);
                  sprintf(tracktext, "%0.2f", gpos.x);
                  sTickText.SetText(tracktext);
                  sTickText.SetPosition(sf::Vector2f(tpos.x+INTEG_SCREEN_IX-16.0f, tpos.y+16.0f+INTEG_SCREEN_IY));
                  ticklinex.SetPosition(sf::Vector2f(tpos.x+INTEG_SCREEN_IX, tpos.y+INTEG_SCREEN_IY));
                  sWindow.Draw(sTickText);
                  sWindow.Draw(ticklinex);

                  tpos.x = winInfo.windims[1].x;
                  tpos.y = k*(winInfo.windims[1].y/winInfo.tickerDensity);
                  MapGraphCoords(winInfo, gpos, tpos, 1);
                  sprintf(tracktext, "%0.2f", -gpos.y);
                  sTickText.SetText(tracktext);
                  sTickText.SetPosition(sf::Vector2f(tpos.x+INTEG_SCREEN_IX+16.0f, tpos.y+INTEG_SCREEN_IY-8.0f));
                  tickliney.SetPosition(sf::Vector2f(tpos.x+INTEG_SCREEN_IX, tpos.y+INTEG_SCREEN_IY));
                  sWindow.Draw(sTickText);
                  sWindow.Draw(tickliney);
            }

            for(k=1;k<ct;k++)
            {
                  Vector2f vposc,vposp,viposc,viposp;
                  MapWindowCoords(winInfo, vposc, deriv[k], 0);
                  MapWindowCoords(winInfo, vposp, deriv[k-1], 0);
                  MapWindowCoords(winInfo, viposc, integ[k], 1);
                  MapWindowCoords(winInfo, viposp, integ[k-1], 1);
                  if(isOnGraph(winInfo, deriv[k], 0))
                  {
                        tline = sf::Shape::Line(DERIV_SCREEN_IX+vposc.x, DERIV_SCREEN_IY+vposc.y, DERIV_SCREEN_IX+vposp.x, DERIV_SCREEN_IY+vposp.y, 1, sf::Color(0.0f, 184.0f, 255.0f));
                        sWindow.Draw(tline);
                  }
                  if(isOnGraph(winInfo, integ[k], 1))
                  {
                        tline = sf::Shape::Line(INTEG_SCREEN_IX+viposc.x, INTEG_SCREEN_IY+viposc.y, INTEG_SCREEN_IX+viposp.x, INTEG_SCREEN_IY+viposp.y, 1, sf::Color(255.0f, 184.0f, 0.0f));
                        sWindow.Draw(tline);
                  }
            }
            sWindow.Draw(sDerivText);
            sWindow.Draw(sFuncText);
		sWindow.Display();
	}
	return 0;
}
