#ifndef _DDRAWTYPES_HPP_
#define _DDRAWTYPES_HPP_

#include "DDataTypes.hpp"
#include <stdio.h>
#include "DParser.hpp"

enum CDDrawType
{
    dtLine = 1,
    dtCircle = 2,
    dtEllipse = 3,
    dtArcEllipse = 4,
    dtHyperbola = 5,
    dtParabola = 6,
    dtSpline = 7,
    dtEvolvent = 8
};

enum CDDrawSubType
{
    dstNone = 0,
    dstRectangle = 1
};

typedef class CDObject
{
private:
    CDDrawType m_iType;
    CDLine m_cLines[2];
    CDRefPoint m_cBounds[2];
    PDPointList m_pInputPoints;
    PDPointList m_pUndoPoints;
    PDPointList m_pCachePoints;
    PDRefList m_pCrossPoints;
    PDPtrList m_pDimens;
    int m_iPrimitiveRequested;

    PDPrimObject m_pPrimitive;
    bool m_bSelected;
    CDLineStyle m_cLineStyle;
    double m_dMovedDist;
    bool m_bFirstDimSet;
    double m_dFirstDimen;
    char m_sTmpDimBuf[64];
    CDDimension m_cTmpDim;
    bool m_bSnapTo;

    int m_iAuxInt;

    int IsClosed();
    bool IsClosedShape();
    void SavePoint(FILE *pf, bool bSwapBytes, CDPoint cPoint);
    void SaveInputPoint(FILE *pf, bool bSwapBytes, CDInputPoint cInPoint);
    void SaveReference(FILE *pf, bool bSwapBytes, double dRef);
    void SaveRefPoint(FILE *pf, bool bSwapBytes, CDRefPoint cRefPoint);
    void SaveLine(FILE *pf, bool bSwapBytes, CDLine cLine);
    void SaveLineStyle(FILE *pf, bool bSwapBytes, CDLineStyle cLineStyle);
    void SaveDimension(FILE *pf, bool bSwapBytes, PDDimension pDim);
    void LoadPoint(FILE *pf, bool bSwapBytes, PDPoint pPoint);
    void LoadInputPoint(FILE *pf, bool bSwapBytes, PDInputPoint pInPoint);
    void LoadReference(FILE *pf, bool bSwapBytes, double *pdRef);
    void LoadRefPoint(FILE *pf, bool bSwapBytes, PDRefPoint pRefPoint);
    void LoadLine(FILE *pf, bool bSwapBytes, PDLine pLine);
    void LoadLineStyle(FILE *pf, bool bSwapBytes, PDLineStyle pLineStyle);
    void LoadDimension(FILE *pf, bool bSwapBytes, PDDimension pDim);
    void AddCurveSegment(double dStart, double dEnd, PDRect pRect);
    void AddPatSegment(double dStart, int iStart, double dEnd, int iEnd,
        PDPoint pBnds, PDRect pRect);
    bool GetNativeReference(double dDist, double *pdRef);
    bool GetNativeRefPoint(double dRef, PDPoint pPt);
    bool GetNativeRefDir(double dRef, PDPoint pPt);
    bool IsValidRef(double dRef);
    bool BoundPoint(CDPoint cRefPt, PDLine pPtX, double *pdDist);
    int AddDimenPrimitive(int iPos, PDDimension pDim, PDPrimObject pPrimitive, PDRect pRect);
    void SwapBounds();
    bool RemovePart(bool bDown, PDRefPoint pBounds);
public:
    CDObject(CDDrawType iType, double dWidth);
    ~CDObject();
    bool AddPoint(double x, double y, char iCtrl, bool bFromGui); // returns true if the point is the last point
    void RemoveLastPoint();
    void Undo();
    void Redo();
    // iMode: 0 - normal, 1 - inserting, 2 - buffering, 3 - rounding
    bool BuildCache(CDLine cTmpPt, int iMode);
    // returns 0 - not in rect, 1 - partially in rect, 2 - full in rect
    int BuildPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, int iTemp, PDFileAttrs pAttrs);
    void GetFirstPrimitive(PDPrimitive pPrim, double dScale, int iDimen);
    void GetNextPrimitive(PDPrimitive pPrim, double dScale, int iDimen);
    int GetBSplineParts();
    // returns true if spline parts is equal to 1, and the spline is closed
    bool GetBSplines(int iParts, double dScale, int *piCtrls, double **ppdKnots, PDPoint *ppPoints);
    bool IsNearPoint(CDPoint cPt, double dTolerance, int *piDimen);
    bool GetSelected();
    void SetSelected(bool bSelect, bool bInvert, int iDimen, PDPtrList pRegions);
    int GetType();
    CDLine GetLine();
    CDLine GetCircle();
    void SetInputLine(int iIndex, CDLine cLine);
    bool HasEnoughPoints();
    bool GetPoint(int iIndex, char iCtrl, PDInputPoint pPoint);
    void SetPoint(int iIndex, char iCtrl, CDInputPoint cPoint);
    double GetDistFromPt(CDPoint cPt, CDPoint cRefPt, bool bSnapCenters, PDLine pPtX, int *piDimen);
    CDLineStyle GetLineStyle();
    void SetLineStyle(int iMask, CDLineStyle cStyle);
    bool GetRestrictPoint(CDPoint cPt, int iMode, bool bRestrictSet, double dRestrictValue,
        PDPoint pSnapPt);
    CDObject* Copy();
    bool Split(CDPoint cPt, double dDist, PDRect pRect, CDObject** ppNewObj, PDPtrList pRegions);
    bool Extend(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions);
    void SetBound(int iIndex, CDLine cBound);
    void SetBound(int iIndex, CDRefPoint cBound);
    void SaveToFile(FILE *pf, bool bSwapBytes);
    bool ReadFromFile(FILE *pf, bool bSwapBytes);
    // pPtX.bIsSet true if center is set, pPtX.cOrigin = center, pPtX.cDirection = normal
    double GetRadiusAtPt(CDLine cPtX, PDLine pPtR, bool bNewPt);
    bool GetDynValue(CDPoint cPt, int iMode, double *pdVal);
    void BuildRound(CDObject *pObj1, CDObject *pObj2, CDPoint cPt, bool bRestSet, double dRad);
    // iDimFlag 0 - don't move dims, 1 - move only selected, 2 - move all dims
    bool RotatePoints(CDPoint cOrig, double dRot, int iDimFlag);
    bool MovePoints(CDPoint cDir, double dDist, int iDimFlag);
    void MirrorPoints(CDLine cLine);
    bool AddCrossPoint(CDPoint cPt, double dDist);
    bool AddCrossPoint(double dRef);
    bool GetPointRefDist(double dRef, double *pdDist);
    double GetNearestCrossPoint(CDPoint cPt, PDPoint pPt);
    double GetNearestBoundPoint(CDPoint cPt, PDPoint pPt);
    bool AddDimen(CDPoint cPt, double dDist, PDRect pRect, PDFileAttrs pAttrs);
    void DiscardDimen();
    void AddDimenPtr(PDDimension pDimen);
    int GetDimenCount();
    PDDimension GetDimen(int iPos);
    int PreParseDimText(int iPos, char *sBuf, int iBufLen, double dScale, PDUnitList pUnits);
    void GetDimFontAttrs(int iPos, PDFileAttrs pAttrs);
    bool DeleteSelDimens(PDRect pRect, PDPtrList pRegions);
    bool GetSelectedDimen(PDDimension pDimen);
    bool SetSelectedDimen(PDDimension pDimen, PDPtrList pRegions);
    bool GetSnapTo();
    void SetSnapTo(bool bSnap);
    void AddRegions(PDPtrList pRegions, int iPrimType);
    int GetUnitMask(int iUnitType, char *psBuf, PDUnitList pUnits);
    bool ChangeUnitMask(int iUnitType, char *psMask, PDUnitList pUnits);
    void Rescale(double dRatio, bool bWidths, bool bPatterns, bool bArrows, bool bLabels);
    CDPoint GetPointToDir(CDPoint cPoint, double dAngle, CDPoint cPtTarget);
    void SetAuxInt(int iVal);
    int GetAuxInt();
    int GetNumParts();
    CDObject* SplitPart(PDRect pRect, PDPtrList pRegions);
} *PDObject;

typedef class CDataList
{
private:
    int m_iDataLen;
    int m_iDataSize;
    PDObject *m_ppObjects;
    CDFileAttrs m_cFileAttrs;
    bool m_bHasChanged;
    int GetTangSnap(CDPoint cPt, double dDist, bool bNewPt, PDLine pSnapPt, PDObject pObj, PDObject pDynObj);
public:
    CDataList();
    ~CDataList();
    int GetCount();
    void Add(PDObject pObject);
    void Remove(int iIndex, bool bFree);
    PDObject GetItem(int iIndex);
    void BuildAllPrimitives(PDRect pRect, bool bResolvePatterns = true);
    PDObject SelectByPoint(CDPoint cPt, double dDist, int *piDimen);
    PDObject SelectLineByPoint(CDPoint cPt, double dDist);
    void ClearSelection(PDPtrList pRegions);
    int GetNumOfSelectedLines();
    PDObject GetSelectedLine(int iIndex);
    int GetNumOfSelectedCircles();
    PDObject GetSelectedCircle(int iIndex);
    int GetSnapPoint(int iSnapMask, CDPoint cPt, double dDist, PDLine pSnapPt, PDObject pDynObj);
    bool DeleteSelected(CDataList *pUndoList, PDRect pRect, PDPtrList pRegions);
    int GetSelectCount();
    PDObject GetSelected(int iIndex);
    bool CutSelected(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions);
    bool ExtendSelected(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions);
    void ClearAll();
    void SaveToFile(FILE *pf, bool bSwapBytes, bool bSelectOnly);
    bool ReadFromFile(FILE *pf, bool bSwapBytes, bool bClear);
    void SelectByRectangle(PDRect pRect, int iMode, PDPtrList pRegions);
    bool RotateSelected(CDPoint cOrig, double dRot, int iCop, PDRect pRect, PDPtrList pRegions);
    bool MoveSelected(CDLine cLine, double dDist, int iCop, PDRect pRect,
        bool bPreserveDir, PDPtrList pRegions);
    bool MirrorSelected(CDLine cLine, PDRect pRect, PDPtrList pRegions);
    int GetSelectedLineStyle(PDLineStyle pStyle);
    bool SetSelectedLineStyle(int iMask, PDLineStyle pStyle, PDPtrList pRegions);
    bool SetCrossSelected(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions);
    bool AddDimen(PDObject pSelForDiment, CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions);
    bool GetChanged();
    void SetChanged();
    void SetFileAttrs(PDFileAttrs pFileAttrs, bool bNewFile);
    void GetFileAttrs(PDFileAttrs pFileAttrs);
    bool GetSelectedDimen(PDDimension pDimen);
    bool SetSelectedDimen(PDDimension pDimen, PDPtrList pRegions);
    void GetStatistics(int *piStats);
    // returns:
    // 0 - success, -1 - no mask found
    int GetUnitMask(int iUnitType, char *psBuf, PDUnitList pUnits);
    void ChangeUnitMask(int iUnitType, char *psMask, PDUnitList pUnits);
    void RescaleDrawing(double dNewScaleNom, double dNewScaleDenom, bool bWidths, bool bPatterns,
        bool bArrows, bool bLabels);
    bool GetSelSnapEnabled();
    void SetSelSnapEnabled(bool bEnable);
    bool BreakSelObjects(PDRect pRect, PDPtrList pRegions);
} *PDataList;

#endif
