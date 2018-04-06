#include "DEllipse.hpp"
#include "DPrimitive.hpp"
#include "DMath.hpp"
#include <math.h>

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddEllipsePoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
{
    if(iCtrl == 2)
    {
        int nOffs = pPoints->GetCount(2);
        if(nOffs > 0) pPoints->SetPoint(0, 2, x, y, iCtrl);
        else pPoints->AddPoint(x, y, iCtrl);
        return true;
    }

    bool bRes = false;
    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);

    if(iInputLines == 2)
    {
        if(iCtrl < 1)
        {
            if(nNorm < 2)
            {
                pPoints->AddPoint(x, y, iCtrl);
                nNorm++;
            }
        }
        bRes = (nNorm > 1);
    }
    else
    {
        if(iCtrl == 1)
        {
            if(nCtrl < 2)
            {
                pPoints->AddPoint(x, y, iCtrl);
                nCtrl++;
            }
        }
        else if(nCtrl > 1)
        {
            if(nNorm < 2)
            {
                pPoints->AddPoint(x, y, iCtrl);
                nNorm++;
            }
        }
        bRes = (nCtrl > 1) && (nNorm > 0);
    }
    return bRes;
}

bool BuildEllipseCache(PDPoint pTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist)
{
    pCache->ClearAll();

    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);

    CDPoint cOrig, cMainDir, cPt1, cPt2, cPt3;
    double d1, d2, d3, da = -1.0, db = -1.0;
    double dr1 = -1.0, dr2 = -1.0;

    if(pLines[0].bIsSet && pLines[1].bIsSet)
    {
        CDPoint cDir1 = pLines[0].cDirection;
        CDPoint cDir2 = pLines[1].cDirection;

        int iX = LineXLine(false, pLines[0].cOrigin, cDir1, pLines[1].cOrigin, cDir2, &cOrig);
        if(iX < 1) return false;

        if(((nNorm < 1) && (iMode == 1)) || ((nNorm == 1) && (iMode != 1)))
        {
            if(iMode == 1) cPt1 = *pTmpPt - cOrig;
            else if(nNorm > 0) cPt1 = pPoints->GetPoint(0, 0).cPoint - cOrig;
            else return false;

            d1 = fabs(Deter2(cDir1, cDir2));
            if(d1 < g_dPrec) return false;

            dr1 = sqrt(Power2(Deter2(cPt1, cDir1)) + Power2(Deter2(cPt1, cDir2)))/d1;
            dr2 = dr1;
        }
        else
        {
            if(nNorm > 0) cPt1 = pPoints->GetPoint(0, 0).cPoint - cOrig;
            else return false;

            if(iMode == 1) cPt2 = *pTmpPt - cOrig;
            else if(nNorm > 1) cPt2 = pPoints->GetPoint(1, 0).cPoint - cOrig;
            else return false;

            CDPoint cMat1, cMat2;
            cMat1.x = Power2(Deter2(cPt1, cDir1));
            cMat1.y = Power2(Deter2(cPt2, cDir1));
            cMat2.x = Power2(Deter2(cPt1, cDir2));
            cMat2.y = Power2(Deter2(cPt2, cDir2));

            db = Power2(Deter2(cDir1, cDir2));
            da = Deter2(cMat1, cMat2);

            d1 = db*(cMat1.x - cMat1.y);
            if(fabs(d1) < g_dPrec) return false;

            d2 = da/d1;
            if(d2 < -g_dPrec) return false;
            if(d2 < g_dPrec) d2 = 0;

            dr1 = sqrt(d2);

            d1 = db*(cMat2.y - cMat2.x);
            if(fabs(d1) < g_dPrec) return false;

            d2 = da/d1;
            if(d2 < -g_dPrec) return false;
            if(d2 < g_dPrec) d2 = 0;

            dr2 = sqrt(d2);
        }

        double dco, dsi;

        if(fabs(dr1 - dr2) < g_dPrec)
        {
            dco = sqrt(2.0)/2.0;
            dsi = dco;
        }
        else
        {
            double dTanT = 2.0*dr1*dr2*(cDir1*cDir2)/(Power2(dr1) - Power2(dr2));
            double dt = atan(dTanT)/2.0;
            dco = cos(dt);
            dsi = sin(dt);
        }
        cMainDir = dco*dr1*cDir1 + dsi*dr2*cDir2;
        da = GetNorm(cMainDir);
        cMainDir /= da;

        d1 = dsi;
        dsi = dco;
        dco = -d1;

        cPt1 = dco*dr1*cDir1 + dsi*dr2*cDir2;
        db = GetNorm(cPt1);

        if(db > da)
        {
            dco = cMainDir.x;
            dsi = cMainDir.y;
            cMainDir.x = -dsi;
            cMainDir.y = dco;
            dsi = da;
            da = db;
            db = dsi;
        }
    }
    else if(nCtrl > 0)
    {
        if(nCtrl < 2)
        {
            cPt1 = pPoints->GetPoint(0, 1).cPoint;
            if(nNorm > 0) cPt2 = pPoints->GetPoint(0, 0).cPoint;
            else if(pTmpPt) cPt2 = *pTmpPt;
            else return false;

            pCache->AddPoint(cPt1.x, cPt1.y, 0);
            pCache->AddPoint(cPt2.x, cPt2.y, 0);
            return true;
        }

        cPt1 = pPoints->GetPoint(0, 1).cPoint;
        cPt2 = pPoints->GetPoint(1, 1).cPoint;

        if(iMode == 1) cPt3 = *pTmpPt;
        else if(nNorm > 0) cPt3 = pPoints->GetPoint(0, 0).cPoint;
        else return false;

        CDPoint cDir = cPt2 - cPt1;
        d1 = GetNorm(cDir);

        if(d1 < g_dPrec) return false;

        d2 = GetDist(cPt1, cPt3);
        d3 = GetDist(cPt2, cPt3);

        da = (d2 + d3)/2.0;
        db = sqrt(Power2(d2 + d3) - Power2(d1))/2.0;

        cMainDir = cDir/d1;
        cOrig = (cPt2 + cPt1)/2.0;
    }

    if((da > g_dPrec) && (db > g_dPrec))
    {
        pCache->AddPoint(cOrig.x, cOrig.y, 0);
        pCache->AddPoint(da, db, 0);
        pCache->AddPoint(cMainDir.x, cMainDir.y, 0);

        int nOffs = pPoints->GetCount(2);
        if((iMode == 2) || (nOffs > 0))
        {
            if(iMode == 2) cPt1 = *pTmpPt;
            else cPt1 = pPoints->GetPoint(0, 2).cPoint;

            CDLine cPtX;
            double dDist = GetElpsDistFromPt(cPt1, cPt1, pCache, &cPtX);
            double dDistOld = 0.0;

            if((nOffs > 0) && (iMode == 2))
            {
                cPt1 = pPoints->GetPoint(0, 2).cPoint;
                dDistOld = GetElpsDistFromPt(cPt1, cPt1, pCache, &cPtX);
            }

            *pdDist = dDist - dDistOld;
            if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);
        }
    }

    return true;
}

int BuildEllipseWithBounds(double da, double db, double dr, double dtStart, double dtEnd,
    CDPoint cOrig, CDPoint cMainDir, PDPrimObject pPrimList, PDRect pRect)
{
    CDPrimitive cPrim, cTmpPrim;
    cPrim.iType = 5;

    if(dtStart > dtEnd) dtEnd += 2.0*M_PI;

    double dAngle = dtEnd - dtStart;
    int iParts = 2 + (int)4.0*dAngle/M_PI;

    CDPoint cPts[5];
    double dt, dt1, dt2, dtDist;
    dtDist = dAngle/(double)iParts;
    dt2 = dtStart;

    CDPoint cDirStart, cDirEnd;
    double dco, dsi;

    dco = cos(dt2);
    dsi = sin(dt2);
    cDirEnd.x = -da*dsi;
    cDirEnd.y = db*dco;
    double dNorm = GetNorm(cDirEnd);

    cPts[4].x = da*dco + dr*cDirEnd.y/dNorm;
    cPts[4].y = db*dsi - dr*cDirEnd.x/dNorm;

    int iRes1 = 2;
    int iRes2 = 0;
    int k;

    for(int i = 0; i < iParts; i++)
    {
        dt1 = dt2;
        dt2 += dtDist;
        cPts[0] = cPts[4];
        cDirStart = cDirEnd;

        for(int j = 1; j < 5; j++)
        {
            dt = dt1 + (dt2 - dt1)*(double)j/4.0;
            dco = cos(dt);
            dsi = sin(dt);

            cDirEnd.x = -da*dsi;
            cDirEnd.y = db*dco;
            dNorm = GetNorm(cDirEnd);

            cPts[j].x = da*dco + dr*cDirEnd.y/dNorm;
            cPts[j].y = db*dsi - dr*cDirEnd.x/dNorm;
        }

        if(ApproxLineSeg(5, cPts, &cDirStart, &cDirEnd, &cTmpPrim) > -0.5)
        {
            cPrim.iType = 5;
            cPrim.cPt1 = cOrig + Rotate(cTmpPrim.cPt1, cMainDir, true);
            cPrim.cPt2 = cOrig + Rotate(cTmpPrim.cPt2, cMainDir, true);
            cPrim.cPt3 = cOrig + Rotate(cTmpPrim.cPt3, cMainDir, true);
            cPrim.cPt4 = cOrig + Rotate(cTmpPrim.cPt4, cMainDir, true);
        }
        else
        {
            cPrim.iType = 1;
            cPrim.cPt1 = cOrig + Rotate(cPts[0], cMainDir, true);
            cPrim.cPt2 = cOrig + Rotate(cPts[4], cMainDir, true);
        }

        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(k < iRes1) iRes1 = 1;
        if(k > iRes2) iRes2 = 1;
    }
    if(iRes2 < 1) iRes1 = 0;
    return iRes1;
}

int BuildEllipseQuadsWithBounds(double da, double db, double dr, double dtStart, double dtEnd,
    CDPoint cOrig, CDPoint cMainDir, PDPrimObject pPrimList, PDRect pRect)
{
    CDPrimitive cPrim, cTmpPrim;
    cPrim.iType = 4;

    if(dtStart > dtEnd) dtEnd += 2.0*M_PI;

    double dAngle = dtEnd - dtStart;
    int iParts = 2 + (int)4.0*dAngle/M_PI;

    double dt2, dtDist;
    dtDist = dAngle/(double)iParts;
    dt2 = dtStart;

    CDPoint cDirStart, cDirEnd;
    double dco, dsi;

    dco = cos(dt2);
    dsi = sin(dt2);
    cDirEnd.x = -da*dsi;
    cDirEnd.y = db*dco;
    double dNorm = GetNorm(cDirEnd);

    cTmpPrim.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
    cTmpPrim.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

    int iRes1 = 2;
    int iRes2 = 0;
    int k;

    for(int i = 0; i < iParts; i++)
    {
        dt2 += dtDist;
        cTmpPrim.cPt1 = cTmpPrim.cPt3;
        cDirStart = cDirEnd;

        dco = cos(dt2);
        dsi = sin(dt2);

        cDirEnd.x = -da*dsi;
        cDirEnd.y = db*dco;
        dNorm = GetNorm(cDirEnd);
        cTmpPrim.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
        cTmpPrim.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;
        LineXLine(false, cTmpPrim.cPt1, cDirStart, cTmpPrim.cPt3, cDirEnd, &cTmpPrim.cPt2);

        cPrim.cPt1 = cOrig + Rotate(cTmpPrim.cPt1, cMainDir, true);
        cPrim.cPt2 = cOrig + Rotate(cTmpPrim.cPt2, cMainDir, true);
        cPrim.cPt3 = cOrig + Rotate(cTmpPrim.cPt3, cMainDir, true);

        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(k < iRes1) iRes1 = 1;
        if(k > iRes2) iRes2 = 1;
    }
    if(iRes2 < 1) iRes1 = 0;
    return iRes1;
}

int BuildEllipsePrimitives(PDPoint pTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(pTmpPt) BuildEllipseCache(pTmpPt, iMode, pPoints, pCache, pLines, pdDist);

    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return 0;

    CDPoint cOrig, cRad, cMainDir;

    CDPrimitive cPrim;
    int iRes = 0;

    if(iCnt < 3)
    {
        cPrim.iType = 1;
        cPrim.cPt1 = pCache->GetPoint(0, 0).cPoint;
        cPrim.cPt2 = pCache->GetPoint(1, 0).cPoint;
        cPrim.cPt3 = 0;
        cPrim.cPt4 = 0;
        pDrawBnds->y = 0;
        pDrawBnds->x = GetDist(cPrim.cPt1, cPrim.cPt2);
        return CropPrimitive(cPrim, pRect, pPrimList);
    }

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dr = dOffset;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr += pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cDir1, cDir2;

    double dLength = 0.0;
    double dco, dsi, dt;

    cPrim.cPt3.x = cRad.x + dr;
    cPrim.cPt3.y = 0.0;
    cDir2.x = 0.0;
    cDir2.y = 1.0;
    for(int i = 0; i < 8; i++)
    {
        cPrim.cPt1 = cPrim.cPt3;
        cDir1 = cDir2;
        dt = M_PI*(i + 1)/16.0;
        dco = cos(dt);
        dsi = sin(dt);
        cDir2.x = -cRad.x*dsi;
        cDir2.y = cRad.y*dco;
        dt = GetNorm(cDir2);
        cPrim.cPt3.x = cRad.x*dco + dr*cDir2.y/dt;
        cPrim.cPt3.y = cRad.y*dsi - dr*cDir2.x/dt;
        LineXLine(false, cPrim.cPt1, cDir1, cPrim.cPt3, cDir2, &cPrim.cPt2);
        dLength += GetQuadLength(&cPrim, 0.0, 1.0);
    }

    pDrawBnds->y = 2*dLength;
    pDrawBnds->x = -pDrawBnds->y;

    cDir1 = cMainDir;
    cDir2.x = -cDir1.y;
    cDir2.y = cDir1.x;

    if(pBounds[0].bIsSet && pBounds[1].bIsSet)
    {
        if(bQuadsOnly)
            iRes = BuildEllipseQuadsWithBounds(cRad.x, cRad.y, dr, pBounds[0].dRef,
                pBounds[1].dRef, cOrig, cMainDir, pPrimList, pRect);
        else
            iRes = BuildEllipseWithBounds(cRad.x, cRad.y, dr, pBounds[0].dRef,
                pBounds[1].dRef, cOrig, cMainDir, pPrimList, pRect);
    }
    else
    {
        if(bQuadsOnly)
            iRes = BuildEllipseQuadsWithBounds(cRad.x, cRad.y, dr, -M_PI, M_PI, cOrig, cMainDir,
                pPrimList, pRect);
        else
            iRes = BuildEllipseWithBounds(cRad.x, cRad.y, dr, -M_PI, M_PI, cOrig, cMainDir,
                pPrimList, pRect);
    }

    return iRes;
}

double ElpProjFn(double da, double db, double dDir, double dx, double dy, double du)
{
    double dv = dDir*sqrt(1.0 - Power2(du));
    return du*dv*(Power2(da) - Power2(db)) - da*dx*du + db*dy*dv;
}

double ElpProjFnDer(double da, double db, double dDir, double dx, double dy, double du)
{
    double dv = dDir*sqrt(1.0 - Power2(du));
    double da1 = Power2(da) - Power2(db);
    return da1*(dv - Power2(du)/dv) - da*dx - db*dy*du/dv;
}

// return cos and sin of dt
CDPoint GetElpsPtProj(double da, double db, CDPoint cPt, CDPoint cRefPt)
{
    double d1 = Power2(da) - Power2(db);
    double dDist2;
    CDPoint cPt1;
    double dNorm, dDist, dDistMin;
    CDPoint cNorm, cNormMin;

    if(fabs(d1) < g_dPrec)
    {
        dDist2 = GetNorm(cPt);
        if(dDist2 > g_dPrec)
        {
            cNormMin = cPt/dDist2;
            cPt1 = da*cNormMin;
            dDistMin = GetDist(cRefPt, cPt1);
            cNorm = -1.0*cNormMin;
            cPt1 = da*cNorm;
            dDist = GetDist(cRefPt, cPt1);
            if(dDist < dDistMin) cNormMin = cNorm;
            return cNormMin;
        }

        dDist2 = GetNorm(cRefPt);
        if(dDist2 > g_dPrec) return cRefPt/dDist2;
        cNormMin.x = 6.0;
        return cNormMin;
    }

    dNorm = sqrt(Power2(cRefPt.x/da) + Power2(cRefPt.y/db));
    if(dNorm < g_dPrec)
    {
        cNormMin.x = 6.0;
        return cNormMin;
    }

    double dPoly[5];
    dPoly[0] = -Power2(db*cPt.y);
    dPoly[1] = -2.0*db*cPt.y*d1;
    dPoly[4] = Power2(d1);
    dPoly[2] = Power2(da*cPt.x) - dPoly[0] - dPoly[4];
    dPoly[3] = -dPoly[1];

    double dRoots[4];
    int iRoots = SolvePolynom(4, dPoly, dRoots);
    if(iRoots < 1)
    {
        cNormMin.x = 6.0;

        int j = 0;
        double du1 = cRefPt.y/db/dNorm;
        double dDir = 1.0;
        if(cRefPt.x < 0) dDir = -1.0;
        double df = ElpProjFn(da, db, dDir, cPt.x, cPt.y, du1);
        double df2 = ElpProjFnDer(da, db, dDir, cPt.x, cPt.y, du1);
        if(fabs(df2) < g_dPrec) return cNormMin;

        double du2 = du1 - df/df2;

        while((j < 8) && (fabs(df) > g_dRootPrec) && (fabs(du2) < 1.0))
        {
            j++;
            du1 = du2;
            df = ElpProjFn(da, db, dDir, cPt.x, cPt.y, du1);
            df2 = ElpProjFnDer(da, db, dDir, cPt.x, cPt.y, du1);
            if(fabs(df2) > g_dPrec) du2 -= df/df2;
            else j = 8;
        }

        if((du2 > 1.0 + g_dPrec) || (du2 < -1.0 - g_dPrec)) return cNormMin;

        if((du2 > 1.0 - g_dPrec) || (du2 < -1.0 + g_dPrec)) du2 = 1.0;

        cNormMin.y = du2;
        cNormMin.x = dDir*sqrt(1.0 - Power2(du2));
        return cNormMin;
    }

    bool bFound = false;
    int i = 0;

    while(!bFound && (i < iRoots))
    {
        cNorm.y = dRoots[i++];
        dDist2 = db*cPt.y + d1*cNorm.y;
        if(fabs(dDist2) > g_dPrec)
        {
            cNorm.x = da*cPt.x*cNorm.y/dDist2;
            dNorm = GetNorm(cNorm);
            bFound = (dNorm > 0.8) && (dNorm < 1.2);
        }
    }

    if(!bFound)
    {
        cNormMin.x = 1.0;
        cNormMin.y = 0.0;
        cPt1.x = da*cNormMin.x;
        cPt1.y = 0.0;
        dDistMin = GetDist(cRefPt, cPt1);
        cPt1.x *= -1.0;
        dDist = GetDist(cRefPt, cPt1);
        if(dDist < dDistMin) cNormMin.x *= -1.0;
        return cNormMin;
    }

    cNormMin = cNorm/dNorm;
    cPt1.x = da*cNormMin.x;
    cPt1.y = db*cNormMin.y;
    dDistMin = GetDist(cRefPt, cPt1);

    while(i < iRoots)
    {
        cNorm.y = dRoots[i++];
        dDist2 = db*cPt.y + d1*cNorm.y;
        if(fabs(dDist2) > g_dPrec)
        {
            cNorm.x = da*cPt.x*cNorm.y/dDist2;
            dNorm = GetNorm(cNorm);
            if((dNorm > 0.8) && (dNorm < 1.2))
            {
                cNorm /= dNorm;
                cPt1.x = da*cNorm.x;
                cPt1.y = db*cNorm.y;
                dDist = GetDist(cRefPt, cPt1);
                if(dDist < dDistMin)
                {
                    cNormMin = cNorm;
                    dDistMin = dDist;
                }
            }
        }
    }
    return cNormMin;
}

double GetElpsDistFromPt(CDPoint cPt, CDPoint cRefPt, PDPointList pCache, PDLine pPtX)
{
    pPtX->bIsSet = false;
    pPtX->dRef = 0.0;

    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return -1.0;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    double dDist = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    CDPoint cRefPt1 = Rotate(cRefPt - cOrig, cMainDir, false);

    CDPoint cNorm = GetElpsPtProj(cRad.x, cRad.y, cPt1, cRefPt1);
    if(cNorm.x > 4.0) return -1.0;

    CDPoint cProjDir, cProjPt;
    double dco, dsi;
    dco = cNorm.x;
    dsi = cNorm.y;

    cProjDir.x = cRad.y*dco;
    cProjDir.y = cRad.x*dsi;
    double dNorm = GetNorm(cProjDir);
    if(dNorm < g_dPrec) return -1.0;

    cProjDir /= dNorm;
    cProjPt.x = cRad.x*dco + dDist*cProjDir.x;
    cProjPt.y = cRad.y*dsi + dDist*cProjDir.y;
    CDPoint cPt2 = Rotate(cRefPt1 - cProjPt, cProjDir, false);
    double dDir = 1.0;
    if(cPt2.x < 0) dDir = -1.0;

    double dRes = GetDist(cRefPt1, cProjPt);

    pPtX->bIsSet = true;
    pPtX->cOrigin = cOrig + Rotate(cProjPt, cMainDir, true);
    pPtX->cDirection = Rotate(cProjDir, cMainDir, true);
    pPtX->dRef = atan2(dsi, dco);

    return dDir*dRes;
}

bool GetElpsRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache)
{
    if(iMode != 2) return false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    double dDist = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.y;

    double dRad = dDist + dRestrictValue;

    CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    CDPoint cNorm = GetElpsPtProj(cRad.x, cRad.y, cPt1, cPt1);

    CDPoint cDir;
    cDir.x = cRad.y*cNorm.x;
    cDir.y = cRad.x*cNorm.y;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return false;

    CDPoint cPt2;
    cPt2.x = cRad.x*cNorm.x + dRad*cDir.x/dNorm;
    cPt2.y = cRad.y*cNorm.y + dRad*cDir.y/dNorm;
    *pSnapPt = cOrig + Rotate(cPt2, cMainDir, true);
    return true;
}

bool HasElpsEnoughPoints(PDPointList pPoints, int iInputLines)
{
    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);
    bool bRes = false;

    if(iInputLines == 2) bRes = (nNorm > 0);
    else bRes = (nCtrl > 1) && (nNorm > 0);

    return bRes;
}

double GetElpsRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt,
    PDPointList pPoints, PDLine pLines)
{
    pPtR->bIsSet = false;
    pPtR->cOrigin = 0;
    pPtR->cDirection = 0;

    PDPointList pLocCache = pCache;
    if(bNewPt)
    {
        pLocCache = new CDPointList();
        double dOff;
        BuildEllipseCache(&cPt, 1, pPoints, pLocCache, pLines, &dOff);
    }

    int iCnt = pLocCache->GetCount(0);

    if(iCnt < 3) return -1.0;

    CDPoint cOrig = pLocCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pLocCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pLocCache->GetPoint(2, 0).cPoint;
    double dDist = 0.0;
    int nOffs = pLocCache->GetCount(2);
    if(nOffs > 0) dDist = pLocCache->GetPoint(0, 2).cPoint.x;

    if(bNewPt) delete pLocCache;

    CDPoint cDir, cPt1, cPt2, cPt3;

    cPt1 = Rotate(cPt - cOrig, cMainDir, false);

    CDPoint cNorm = GetElpsPtProj(cRad.x, cRad.y, cPt1, cPt1);
    if(cNorm.x > 4.0) return -1.0;
    cPt3.x = cRad.x*cNorm.x;
    cPt3.y = cRad.y*cNorm.y;

    cDir.x = cRad.y*cNorm.x;
    cDir.y = cRad.x*cNorm.y;
    double dNorm = GetNorm(cDir);

    double da2 = Power2(cRad.x);
    double db2 = Power2(cRad.y);
    cPt2.x = (da2 - db2)*Power3(cPt3.x)/Power2(da2);
    cPt2.y = (db2 - da2)*Power3(cPt3.y)/Power2(db2);

    double dRad = GetDist(cPt3, cPt2);

    pPtR->bIsSet = true;
    pPtR->cOrigin = cOrig + Rotate(cPt2, cMainDir, true);
    if(dNorm > g_dPrec)
    pPtR->cDirection = Rotate(cDir/dNorm, cMainDir, true);

    return dRad + dDist;
}

bool GetElpsPointRefDist(double dRef, PDPointList pCache, double *pdDist)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    double dDist = 0.0;

    if(iCnt < 3)
    {
        dDist = GetDist(cRad, cOrig);
        *pdDist = dRef*dDist;
        return true;
    }

    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    //CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.x;

    //CDPoint cNorm = GetElpsPtProj(cRad.x, cRad.y, cPt1, cPt1);
    //if(cNorm.x > 4.0) return false;
    double dAng = dRef; //atan2(cNorm.y, cNorm.x);
    double dDir = 1.0;
    if(dAng < 0.0)
    {
        dDir = -1.0;
        dAng *= dDir;
    }

    double dStep = M_PI/8.0;
    int iSteps = 1 + dAng/dStep;
    dStep = dAng/iSteps;

    CDPrimitive cQuad;
    double dt = dStep;
    CDPoint cDirStart;
    CDPoint cDirEnd = {0, 1.0};
    cQuad.cPt3.x = cRad.x + dDist;
    cQuad.cPt3.y = 0.0;

    double dRes = 0.0;
    double dco, dsi, dNorm;

    for(int i = 0; i < iSteps; i++)
    {
        dco = cos(dt);
        dsi = sin(dt);

        cDirStart = cDirEnd;
        cDirEnd.x = -cRad.x*dsi;
        cDirEnd.y = cRad.y*dco;
        dNorm = GetNorm(cDirEnd);

        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt3.x = cRad.x*dco + dDist*cDirEnd.y/dNorm;
        cQuad.cPt3.y = cRad.y*dsi - dDist*cDirEnd.x/dNorm;

        LineXLine(false, cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

        dRes += GetQuadLength(&cQuad, 0.0, 1.0);

        dt += dStep;
    }

    *pdDist = dDir*dRes;
    return true;
}

double GetElpsPointAtDist(double da, double db, double dr, double dDist)
{
    double dStep = M_PI/8;
    double dDir = 1.0;
    if(dDist < 0.0)
    {
        dDir = -1.0;
        dDist *= dDir;
    }
    bool bFound = false;

    CDPrimitive cQuad;
    double dt = 0.0;
    CDPoint cDirStart;
    CDPoint cDirEnd = {0, 1.0};
    cQuad.cPt3.x = da + dr;
    cQuad.cPt3.y = 0.0;
    double d1, dco, dsi, dNorm;

    while(!bFound)
    {
        dt += dStep;

        dco = cos(dt);
        dsi = sin(dt);

        cDirStart = cDirEnd;
        cDirEnd.x = -da*dsi;
        cDirEnd.y = db*dco;
        dNorm = GetNorm(cDirEnd);

        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
        cQuad.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

        LineXLine(false, cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

        d1 = GetQuadLength(&cQuad, 0.0, 1.0);
        if(d1 < dDist) dDist -= d1;
        else bFound = true;
    }

    double dt1 =  GetQuadPointAtDist(&cQuad, 0.0, dDist);
    CDPoint cPt1 = GetQuadPoint(&cQuad, dt1);
    cPt1.y *= dDir;

    CDPoint cNorm = GetElpsPtProj(da, db, cPt1, cPt1);
    return atan2(cNorm.y, cNorm.x);
}

void AddElpsSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    if(iCnt < 3)
    {
        CDPrimitive cPrim;
        CDPoint cPt1;
        CDPoint cDir = cRad - cOrig;
        double dNorm = GetNorm(cDir);
        if(dNorm < g_dPrec) return;

        cDir /= dNorm;
        cPt1.x = d1;
        cPt1.y = 0.0;

        cPrim.iType = 1;
        cPrim.cPt1 = cOrig + Rotate(cPt1, cDir, true);
        cPt1.x = d2;
        cPrim.cPt2 = cOrig + Rotate(cPt1, cDir, true);
        cPrim.cPt3 = 0;
        cPrim.cPt4 = 0;
        CropPrimitive(cPrim, pRect, pPrimList);
        return;
    }

    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dt1 = GetElpsPointAtDist(cRad.x, cRad.y, dr, d1);
    double dt2 = GetElpsPointAtDist(cRad.x, cRad.y, dr, d2);
    BuildEllipseWithBounds(cRad.x, cRad.y, dr, dt1, dt2, cOrig, cMainDir, pPrimList, pRect);
}

bool GetElpsRefPoint(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dco = cos(dRef);
    double dsi = sin(dRef);

    CDPoint cNorm;
    cNorm.x = cRad.y*dco;
    cNorm.y = cRad.x*dsi;
    double dN1 = GetNorm(cNorm);
    if(dN1 < g_dPrec) return false;

    CDPoint cPt1;
    cPt1.x = cRad.x*dco + dr*cNorm.x/dN1;
    cPt1.y = cRad.y*dsi + dr*cNorm.y/dN1;
    *pPt = cOrig + Rotate(cPt1, cMainDir, true);
    return true;
}

bool GetElpsRefDir(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dco = cos(dRef);
    double dsi = sin(dRef);

    CDPoint cNorm;
    cNorm.x = -cRad.x*dsi;
    cNorm.y = cRad.y*dco;
    double dN1 = GetNorm(cNorm);
    if(dN1 < g_dPrec) return false;

    CDPoint cPt1;
    cPt1.x = cNorm.x/dN1;
    cPt1.y = cNorm.y/dN1;
    *pPt = Rotate(cPt1, cMainDir, true);
    return true;
}

bool GetElpsReference(double dDist, PDPointList pCache, double *pdRef)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    *pdRef = GetElpsPointAtDist(cRad.x, cRad.y, dr, dDist);
    return true;
}

