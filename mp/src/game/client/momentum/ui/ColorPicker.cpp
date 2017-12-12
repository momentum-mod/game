#include "cbase.h"

#include "util/mom_util.h"
#include "vgui/IInput.h"
#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/KeyCode.h"
#include "vgui/MouseCode.h"

#include "ColorPicker.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/Slider.h"

#include "KeyValues.h"

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define __EXTRUDE_BORDER 3

void HSV2RGB(float H, float s, float v, Vector &normalizedRGB)
{
    int Hr = floor(H / 60.0f);
    float f = H / 60.0f - Hr;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (Hr)
    {
    default:
        normalizedRGB.Init(v, t, p);
        break;
    case 1:
        normalizedRGB.Init(q, v, p);
        break;
    case 2:
        normalizedRGB.Init(p, v, t);
        break;
    case 3:
        normalizedRGB.Init(p, q, v);
        break;
    case 4:
        normalizedRGB.Init(t, p, v);
        break;
    case 5:
        normalizedRGB.Init(v, p, q);
        break;
    }
}

void HSV2RGB(float H, float s, float v, Vector4D &normalizedRGBA)
{
    Vector tmp;
    HSV2RGB(H, s, v, tmp);
    normalizedRGBA.Init(tmp.x, tmp.y, tmp.z, normalizedRGBA.w);
}

void HSV2RGB(const Vector &hsv, Vector4D &normalizedRGBA) { HSV2RGB(hsv.x, hsv.y, hsv.z, normalizedRGBA); }
void HSV2RGB(const Vector &hsv, Vector &normalizedRGBA) { HSV2RGB(hsv.x, hsv.y, hsv.z, normalizedRGBA); }
void RGB2HSV(const Vector4D &normalizedRGB, float &H, float &s, float &v)
{
    float fmax = max(normalizedRGB.x, max(normalizedRGB.y, normalizedRGB.z));
    float fmin = min(normalizedRGB.x, min(normalizedRGB.y, normalizedRGB.z));

    v = fmax;

    if (fmax <= 0.0f || normalizedRGB.LengthSqr() <= 0.0f)
        s = 0.0f;
    else
        s = (fmax - fmin) / fmax;

    if (fmax == fmin || (normalizedRGB.x == normalizedRGB.y && normalizedRGB.x == normalizedRGB.z))
    {
        H = 0; //-1.0f;
    }
    else if (normalizedRGB.x >= fmax)
        H = 60.0f * ((normalizedRGB.y - normalizedRGB.z) / (fmax - fmin));
    else if (normalizedRGB.y >= fmax)
        H = 60.0f * (2 + (normalizedRGB.z - normalizedRGB.x) / (fmax - fmin));
    else if (normalizedRGB.z >= fmax)
        H = 60.0f * (4 + (normalizedRGB.x - normalizedRGB.y) / (fmax - fmin));
    if (H < 0)
        H += 360.0f;
}

void RGB2HSV(const Vector4D &normalizedRGB, Vector &hsv) { RGB2HSV(normalizedRGB, hsv.x, hsv.y, hsv.z); }

void SetupVguiTex(int &var, const char *tex)
{
    var = surface()->DrawGetTextureId(tex);
    if (var <= 0)
    {
        var = surface()->CreateNewTextureID();
        surface()->DrawSetTextureFile(var, tex, true, false);
    }
}

void Vec4DToColor(const Vector4D &vector, Color &into)
{
    into.SetColor(vector.x * 255, vector.y * 255, vector.z * 255, vector.w * 255);
}

DECLARE_BUILD_FACTORY(HSV_Select_Base);

HSV_Select_Base::HSV_Select_Base(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
{
    if (parent)
        AddActionSignalTarget(parent->GetVPanel());

    m_bIsReading = false;
    SetupVguiTex(m_iMat, "shadereditor/colorpicker");
}

void HSV_Select_Base::OnMousePressed(MouseCode code)
{
    BaseClass::OnMousePressed(code);
    if (code == MOUSE_LEFT)
    {
        m_bIsReading = true;
        input()->SetMouseCapture(GetVPanel());
        ReadValues();
    }
}
void HSV_Select_Base::OnMouseReleased(MouseCode code)
{
    BaseClass::OnMouseReleased(code);
    if (code == MOUSE_LEFT && m_bIsReading)
    {
        m_bIsReading = false;
        input()->SetMouseCapture(0);
    }
}
void HSV_Select_Base::OnMouseCaptureLost()
{
    BaseClass::OnMouseCaptureLost();
    m_bIsReading = false;
}
void HSV_Select_Base::OnCursorMoved(int x, int y)
{
    BaseClass::OnCursorMoved(x, y);
    if (m_bIsReading)
        ReadValues();
}
void HSV_Select_Base::ReadValues() { PostActionSignal(new KeyValues("HSVUpdate")); }

DECLARE_BUILD_FACTORY(HSV_Select_SV);

HSV_Select_SV::HSV_Select_SV(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
{
    m_flH = 0;
    m_flS = 0;
    m_flV = 0;
}
void HSV_Select_SV::Paint()
{
    surface()->DrawSetTexture(m_iMat);
    surface()->DrawSetColor(Color(255, 255, 255, 255));
    int x, y, sx, sy;
    GetBounds(x, y, sx, sy);

    Vertex_t points[4];
    points[0].m_TexCoord.Init(0, 0);
    points[1].m_TexCoord.Init(1, 0);
    points[2].m_TexCoord.Init(1, 1);
    points[3].m_TexCoord.Init(0, 1);

    IMaterial *pMatColorpicker = materials->FindMaterial("shadereditor/colorpicker", TEXTURE_GROUP_OTHER);
    if (IsErrorMaterial(pMatColorpicker))
        return;
    bool bFound = false;
    IMaterialVar *pVar_00 = pMatColorpicker->FindVar("$COLOR_00", &bFound);
    IMaterialVar *pVar_10 = pMatColorpicker->FindVar("$COLOR_10", &bFound);
    IMaterialVar *pVar_11 = pMatColorpicker->FindVar("$COLOR_11", &bFound);
    IMaterialVar *pVar_01 = pMatColorpicker->FindVar("$COLOR_01", &bFound);
    if (!bFound)
        return;

    Vector col;
    HSV2RGB(m_flH, 1, 1, col);

    pVar_00->SetVecValue(1, 1, 1);
    pVar_10->SetVecValue(col.Base(), 3);
    pVar_11->SetVecValue(0, 0, 0);
    pVar_01->SetVecValue(0, 0, 0);

    surface()->DrawTexturedRect(0, 0, sx, sy);
}
void HSV_Select_SV::ReadValues()
{
    int mx, my;
    input()->GetCursorPosition(mx, my);
    ScreenToLocal(mx, my);

    int sx, sy;
    GetSize(sx, sy);

    int vpos = clamp(sy - my, 0, sy);
    m_flV = vpos / static_cast<float>(sy);

    int spos = sx - clamp(sx - mx, 0, sx);
    m_flS = spos / static_cast<float>(sx);

    BaseClass::ReadValues();
}

DECLARE_BUILD_FACTORY(HSV_Select_Hue);

HSV_Select_Hue::HSV_Select_Hue(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
{
    m_flHue = 0;
}
void HSV_Select_Hue::Paint()
{
    surface()->DrawSetTexture(m_iMat);
    surface()->DrawSetColor(Color(255, 255, 255, 255));
    int x, y, sx, sy;
    GetBounds(x, y, sx, sy);

    Vector rgbs[7];
    for (int i = 0; i < 7; i++)
        HSV2RGB(i * 60.0f, 1, 1, rgbs[i]);

    Vertex_t points[4];
    points[0].m_TexCoord.Init(0, 0);
    points[1].m_TexCoord.Init(1, 0);
    points[2].m_TexCoord.Init(1, 1);
    points[3].m_TexCoord.Init(0, 1);

    IMaterial *pMatColorpicker = materials->FindMaterial("shadereditor/colorpicker", TEXTURE_GROUP_OTHER);
    if (IsErrorMaterial(pMatColorpicker))
        return;
    bool bFound = false;
    IMaterialVar *pVar_00 = pMatColorpicker->FindVar("$COLOR_00", &bFound);
    IMaterialVar *pVar_10 = pMatColorpicker->FindVar("$COLOR_10", &bFound);
    IMaterialVar *pVar_11 = pMatColorpicker->FindVar("$COLOR_11", &bFound);
    IMaterialVar *pVar_01 = pMatColorpicker->FindVar("$COLOR_01", &bFound);
    if (!bFound)
        return;

    int delta_y = sy / 6;
    int cur_pos_y = sy;

    for (int i = 0; i < 6; i++)
    {
        if (i == 5)
            delta_y = cur_pos_y;

        pVar_00->SetVecValue(rgbs[i + 1].Base(), 3);
        pVar_10->SetVecValue(rgbs[i + 1].Base(), 3);
        pVar_11->SetVecValue(rgbs[i].Base(), 3);
        pVar_01->SetVecValue(rgbs[i].Base(), 3);

        points[0].m_Position.Init(0, cur_pos_y - delta_y);
        points[1].m_Position.Init(sx, cur_pos_y - delta_y);
        points[2].m_Position.Init(sx, cur_pos_y);
        points[3].m_Position.Init(0, cur_pos_y);

        surface()->DrawTexturedPolygon(4, points);
        cur_pos_y -= delta_y;
    }
}
void HSV_Select_Hue::ReadValues()
{
    int mx, my;
    input()->GetCursorPosition(mx, my);
    ScreenToLocal(mx, my);

    int sx, sy;
    GetSize(sx, sy);

    int huepos = clamp(sy - my, 0, sy);
    m_flHue = (huepos / static_cast<float>(sy)) * 360.0f;
    m_flHue = clamp(m_flHue, 0.0f, 360.0f);

    BaseClass::ReadValues();
}

class PickerHelper : public Panel
{
  public:
    DECLARE_CLASS_SIMPLE(PickerHelper, Panel);
    PickerHelper(int img, Panel *parent) : Panel(parent, "")
    {
        m_iImage = img;
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
        SetPaintBackgroundEnabled(false);
    };

    int m_iImage;

    void Paint() OVERRIDE
    {
        surface()->DrawSetColor(Color(255, 255, 255, 255));
        surface()->DrawSetTexture(m_iImage);
        int sx, sy;
        GetSize(sx, sy);
        surface()->DrawTexturedRect(0, 0, sx, sy);
    };
    void OnThink() OVERRIDE { SetZPos(100); };
    void SetCenter(int x, int y)
    {
        int sx, sy;
        GetSize(sx, sy);
        SetPos(x - sx / 2, y - sy / 2);
    };
};

ColorPicker::ColorPicker(Panel *parent, Panel *pActionsignalTarget) : BaseClass(parent, "CColorPicker")
{
    pTarget = nullptr;
    pTargetCallback = nullptr;
    AddActionSignalTarget(pActionsignalTarget);
    Init();
}
ColorPicker::ColorPicker(Panel *parent, TextEntry *pTargetEntry) : BaseClass(parent, "CColorPicker")
{
    pTarget = pTargetEntry;
    pTargetCallback = nullptr;
    Init();
}

ColorPicker::~ColorPicker()
{
    if (pDrawPicker_Hue)
        pDrawPicker_Hue->DeletePanel();

    if (pDrawPicker_SV)
        pDrawPicker_SV->DeletePanel();

    pDrawPicker_Hue = nullptr;
    pDrawPicker_SV = nullptr;
    pTarget = nullptr;
    pTargetCallback = nullptr;
}

void ColorPicker::Init()
{
    SetProportional(false);
    m_vecColor.Init(0, 0, 0, 1);
    m_vecHSV.Init();

    if (pTarget)
    {
        char buf[MAX_PATH];
        pTarget->GetText(buf, MAX_PATH);
        CSplitString tokens(buf, " ");
        for (int i = 0; i < tokens.Count() && i < 3; i++)
            m_vecColor[i] = clamp(Q_atof(tokens[i]), 0.0f, 1.0f);
        RGB2HSV(m_vecColor, m_vecHSV);
    }

    LoadControlSettings("resource/ui/ColorPicker.res");

    m_pColorPreview = FindControl<Panel>("colorpreview");
    if (m_pColorPreview)
    {
        m_pColorPreview->SetPaintEnabled(false);
        m_pColorPreview->SetPaintBackgroundEnabled(false);
    }

    m_pSelect_Hue = FindControl<HSV_Select_Hue>("pick_hue");
    m_pSelect_SV = FindControl<HSV_Select_SV>("pick_sv");
    m_pText_HEX = FindControl<TextEntry>("col_hex");

    m_pAlphaSlider = FindControl<Slider>("AlphaSlider");
    if (m_pAlphaSlider)
        m_pAlphaSlider->SetValue(255, false);

    for (int i = 0; i < 4; i++)
    {
        char tentry_name[64];
        Q_snprintf(tentry_name, 64, "col_%i", i);
        m_pText_RGBA[i] = FindControl<TextEntry>(tentry_name);
    }

    SetFadeEffectDisableOverride(true);
    SetVisible(false);
    SetSizeable(false);
    SetSize(440, 300);
    SetTitle("Color Picker", false);

    SetupVguiTex(m_iVgui_Pick_Hue, "shadereditor/colorpicker_hue");
    SetupVguiTex(m_iVgui_Pick_SV, "shadereditor/colorpicker_sv");

    pDrawPicker_Hue = new PickerHelper(m_iVgui_Pick_Hue, this);
    pDrawPicker_Hue->SetSize(48, 16);
    pDrawPicker_SV = new PickerHelper(m_iVgui_Pick_SV, this);
    pDrawPicker_SV->SetSize(16, 16);

    UpdateAllVars();
}

void ColorPicker::Show()
{
    DoModal();
    MakeReadyForUse();
    InvalidateLayout(true, true);
}

void ColorPicker::Paint()
{
    Color bg = GetBgColor();
    bg[3] = 160;
    SetBgColor(bg);

    BaseClass::Paint();

    surface()->DrawSetColor(Color(0, 0, 0, 255));

    int x, y, sx, sy;
    m_pSelect_Hue->GetBounds(x, y, sx, sy);
    x -= __EXTRUDE_BORDER;
    y -= __EXTRUDE_BORDER;
    sx += __EXTRUDE_BORDER * 2;
    sy += __EXTRUDE_BORDER * 2;
    surface()->DrawFilledRect(x, y, x + sx, y + sy);
    m_pSelect_SV->GetBounds(x, y, sx, sy);
    x -= __EXTRUDE_BORDER;
    y -= __EXTRUDE_BORDER;
    sx += __EXTRUDE_BORDER * 2;
    sy += __EXTRUDE_BORDER * 2;
    surface()->DrawFilledRect(x, y, x + sx, y + sy);

    if (m_pColorPreview)
    {
        m_pColorPreview->GetBounds(x, y, sx, sy);
        surface()->DrawSetColor(Color(m_vecColor.x * 255, m_vecColor.y * 255, m_vecColor.z * 255, m_vecColor.w * 255));
        surface()->DrawFilledRect(x, y, x + sx, y + sy);

        x -= __EXTRUDE_BORDER;
        y -= __EXTRUDE_BORDER;
        sx += __EXTRUDE_BORDER * 2;
        sy += __EXTRUDE_BORDER * 2;
        surface()->DrawFilledRect(x, y, x + sx, y + sy);
    }
}

void ColorPicker::OnThink()
{
    int x, y, sx, sy;
    m_pSelect_Hue->GetBounds(x, y, sx, sy);
    float hue = clamp((360.0f - m_pSelect_Hue->GetHue()) / 360.0f, 0.0f, 360.0f);
    pDrawPicker_Hue->SetCenter(x + sx / 2, y + sy * hue);

    m_pSelect_SV->GetBounds(x, y, sx, sy);
    float _s = m_pSelect_SV->GetS();
    float _v = 1.0f - m_pSelect_SV->GetV();
    pDrawPicker_SV->SetCenter(x + sx * _s, y + sy * _v);
}

void ColorPicker::SetPickerColor(const Color &col)
{
    for (int i = 0; i < 4; i++)
    {
        m_vecColor[i] = static_cast<float>(col[i]) / 255.0f;
    }

    SetPickerColor(m_vecColor);
}

void ColorPicker::SetPickerColor(const Vector4D &col)
{
    for (int i = 0; i < 4; i++)
        m_vecColor[i] = clamp(col[i], 0.0f, 1.0f);

    RGB2HSV(m_vecColor, m_vecHSV);

    UpdateAllVars();
}
Vector4D ColorPicker::GetPickerColor() const { return m_vecColor; }
void ColorPicker::SetPickerColorHSV(const Vector &col)
{
    m_vecHSV = col;
    HSV2RGB(m_vecHSV, m_vecColor);
    UpdateAllVars();
}
Vector ColorPicker::GetPickerColorHSV() const { return m_vecHSV; }
void ColorPicker::ApplySchemeSettings(IScheme *pScheme) { BaseClass::ApplySchemeSettings(pScheme); }

void ColorPicker::UpdateAlpha(bool bWasSlider)
{
    int newValue;

    if (bWasSlider)
    {
        // Just in case
        newValue = clamp<int>(m_pAlphaSlider->GetValue(), 0, 255);
        char tmpText[10];
        Q_snprintf(tmpText, 10, "%03i", newValue);
        m_pText_RGBA[3]->SetText(tmpText);
    }
    else
    {
        char tmpText[32];
        m_pText_RGBA[3]->GetText(tmpText, 32);
        newValue = clamp<int>(Q_atoi(tmpText), 0, 255);
        m_pAlphaSlider->SetValue(newValue, false);
    }

    m_vecColor[3] = static_cast<float>(newValue) / 255.0f;
    Panel *pIgnore;
    if (bWasSlider)
        pIgnore = m_pAlphaSlider;
    else 
        pIgnore = m_pText_RGBA[3];
    UpdateAllVars(pIgnore);
}

void ColorPicker::OnCommand(const char *cmd)
{
    if (FStrEq(cmd, "save"))
    {
        if (pTarget)
        {
            char str[MAX_PATH];
            Q_snprintf(str, MAX_PATH, "%f %f %f", m_vecColor.x, m_vecColor.y, m_vecColor.z);
            pTarget->SetText(str);
        }
        else
        {
            KeyValues *pKV = new KeyValues("ColorSelected");
            Color picked;
            Vec4DToColor(m_vecColor, picked);
            pKV->SetColor("color", picked);
            pKV->SetPtr("targetCallback", pTargetCallback);
            PostActionSignal(pKV);
        }
        Close();
        return;
    }
    BaseClass::OnCommand(cmd);
}

void ColorPicker::OnHSVUpdate(Panel *pPanel)
{
    m_vecHSV.x = m_pSelect_Hue->GetHue();
    m_vecHSV.y = m_pSelect_SV->GetS();
    m_vecHSV.z = m_pSelect_SV->GetV();

    HSV2RGB(m_vecHSV.x, m_vecHSV.y, m_vecHSV.z, m_vecColor);
    UpdateAllVars(pPanel);
}

void ColorPicker::OnSliderMoved(Panel *panel)
{
    if (panel == m_pAlphaSlider)
    {
        UpdateAlpha(true);
    }
}

void ColorPicker::OnTextChanged(Panel *pPanel)
{
    TextEntry *pTEntry = dynamic_cast<TextEntry *>(pPanel);

    Assert(pTEntry);

    char tmpText[32];
    pTEntry->GetText(tmpText, 32);

    // If we're changing alpha, we only update our alpha and move on
    if (pTEntry == m_pText_RGBA[3])
    {
        UpdateAlpha(false);
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        if (pTEntry == m_pText_RGBA[i])
            m_vecColor[i] = static_cast<float>(Q_atoi(tmpText)) / 255.0f;
    }

    if (pTEntry == m_pText_HEX)
    {
        Color picked;
        if (g_pMomentumUtil->GetColorFromHex(tmpText, picked))
        {
            for (int i = 0; i < 4; i++)
            {
                m_vecColor[i] = static_cast<float>(picked[i]) / 255.0f;
            }
        }
        
    }

    RGB2HSV(m_vecColor, m_vecHSV);
    UpdateAllVars(pTEntry);
}

void ColorPicker::UpdateAllVars(Panel *pIgnore)
{
    for (int i = 0; i < 4; i++)
        if (pIgnore != m_pText_RGBA[i])
        {
            char tmp[64];
            Q_snprintf(tmp, 64, "%03i", static_cast<int>(clamp(m_vecColor[i] * 255.0f, 0.0f, 255.0f)));
            m_pText_RGBA[i]->SetText(tmp);
        }

    if (pIgnore != m_pText_HEX)
    {
        char hexString[9];
        Color currentColor;
        Vec4DToColor(m_vecColor, currentColor);
        g_pMomentumUtil->GetHexStringFromColor(currentColor, hexString, 9);
        m_pText_HEX->SetText(hexString);
    }

    if (pIgnore != m_pAlphaSlider)
        m_pAlphaSlider->SetValue(clamp<int>(m_vecColor[3] * 255.0f, 0, 255), false);

    if (pIgnore != m_pSelect_Hue)
        m_pSelect_Hue->SetHue(clamp(m_vecHSV.x, 0.0f, 360.0f));
    if (pIgnore != m_pSelect_SV)
    {
        m_pSelect_SV->SetSV(m_vecHSV.y, m_vecHSV.z);
        m_pSelect_SV->SetHue(m_vecHSV.x);
    }
}