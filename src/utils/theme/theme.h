// Copyright (C) 2016 Thorben Kroeger <thorbenkroeger@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "../utils_global.h"

#include <utils/result.h>

#include <QBrush> // QGradientStops
#include <QObject>

QT_BEGIN_NAMESPACE
class QMenu;
class QPalette;
class QSettings;
QT_END_NAMESPACE

namespace Utils {

namespace StyleHelper {
enum class ToolbarStyle;
}

class ThemePrivate;

class UTILS_EXPORT Theme : public QObject
{
    Q_OBJECT
public:
    Theme(const QString &id, QObject *parent = nullptr);
    ~Theme() override;

    enum Color {
        BackgroundColorDark,
        BackgroundColorHover,
        BackgroundColorNormal,
        BackgroundColorSelected,
        BackgroundColorDisabled,
        BadgeLabelBackgroundColorChecked,
        BadgeLabelBackgroundColorUnchecked,
        BadgeLabelTextColorChecked,
        BadgeLabelTextColorUnchecked,
        ComboBoxTextColor,
        DetailsWidgetBackgroundColor,
        DockWidgetResizeHandleColor,
        EditorPlaceholderColor,
        FancyToolBarSeparatorColor,
        FancyTabBarSelectedBackgroundColor,
        FancyTabWidgetDisabledSelectedTextColor,
        FancyTabWidgetDisabledUnselectedTextColor,
        FancyTabWidgetEnabledSelectedTextColor,
        FancyTabWidgetEnabledUnselectedTextColor,
        FancyToolButtonHoverColor,
        FancyToolButtonSelectedColor,
        FancyToolButtonHighlightColor,
        InfoBarBackground,
        InfoBarText,
        MenuBarItemTextColorDisabled,
        MenuBarItemTextColorNormal,
        MenuItemTextColorDisabled,
        MenuItemTextColorNormal,
        MiniProjectTargetSelectorBackgroundColor,
        MiniProjectTargetSelectorBorderColor,
        MiniProjectTargetSelectorTextColor,
        OutputPaneButtonFlashColor,
        OutputPaneToggleButtonTextColorChecked,
        OutputPaneToggleButtonTextColorUnchecked,
        PanelStatusBarBackgroundColor,
        PanelTextColorDark,
        PanelTextColorMid,
        PanelTextColorLight,
        ProgressBarColorError,
        ProgressBarColorFinished,
        ProgressBarColorNormal,
        ProgressBarTitleColor,
        ProgressBarBackgroundColor,
        SplitterColor,
        TextColorDisabled,
        TextColorError,
        TextColorHighlightBackground,
        TextColorLink,
        TextColorNormal,

        /* Palette for QPalette */

        PaletteWindow,
        PaletteWindowText,
        PaletteBase,
        PaletteAlternateBase,
        PaletteToolTipBase,
        PaletteToolTipText,
        PaletteText,
        PaletteButton,
        PaletteButtonText,
        PaletteBrightText,
        PaletteHighlight,
        PaletteHighlightedText,
        PaletteLink,
        PaletteLinkVisited,

        PaletteLight,
        PaletteMidlight,
        PaletteDark,
        PaletteMid,
        PaletteShadow,

        PaletteWindowDisabled,
        PaletteWindowTextDisabled,
        PaletteBaseDisabled,
        PaletteAlternateBaseDisabled,
        PaletteToolTipBaseDisabled,
        PaletteToolTipTextDisabled,
        PaletteTextDisabled,
        PaletteButtonDisabled,
        PaletteButtonTextDisabled,
        PaletteBrightTextDisabled,
        PaletteHighlightDisabled,
        PaletteHighlightedTextDisabled,
        PaletteLinkDisabled,
        PaletteLinkVisitedDisabled,

        PaletteLightDisabled,
        PaletteMidlightDisabled,
        PaletteDarkDisabled,
        PaletteMidDisabled,
        PaletteShadowDisabled,

        PalettePlaceholderText,
        PalettePlaceholderTextDisabled,
        PaletteAccent,
        PaletteAccentDisabled,

        /* Icons */

        IconsBaseColor,
        IconsDisabledColor,
        IconsInfoColor,
        IconsInfoToolBarColor,
        IconsWarningColor,
        IconsWarningToolBarColor,
        IconsErrorColor,
        IconsErrorToolBarColor,
        IconsRunColor,
        IconsRunToolBarColor,
        IconsStopColor,
        IconsStopToolBarColor,
        IconsInterruptColor,
        IconsInterruptToolBarColor,
        IconsDebugColor,
        IconsNavigationArrowsColor,
        IconsBuildHammerHandleColor,
        IconsBuildHammerHeadColor,

        /* Code model Icons */

        IconsCodeModelKeywordColor,
        IconsCodeModelClassColor,
        IconsCodeModelStructColor,
        IconsCodeModelFunctionColor,
        IconsCodeModelVariableColor,
        IconsCodeModelEnumColor,
        IconsCodeModelMacroColor,
        IconsCodeModelAttributeColor,
        IconsCodeModelUniformColor,
        IconsCodeModelVaryingColor,
        IconsCodeModelOverlayBackgroundColor,
        IconsCodeModelOverlayForegroundColor,

        /* Code model text marks */

        CodeModel_Error_TextMarkColor,
        CodeModel_Warning_TextMarkColor,
        CodeModel_Info_TextMarkColor,

        /* Output panes */

        OutputPanes_DebugTextColor,
        OutputPanes_ErrorMessageTextColor,
        OutputPanes_MessageOutput,
        OutputPanes_NormalMessageTextColor,
        OutputPanes_StdErrTextColor,
        OutputPanes_StdOutTextColor,
        OutputPanes_WarningMessageTextColor,
        OutputPanes_TestPassTextColor,
        OutputPanes_TestFailTextColor,
        OutputPanes_TestXFailTextColor,
        OutputPanes_TestXPassTextColor,
        OutputPanes_TestSkipTextColor,
        OutputPanes_TestWarnTextColor,
        OutputPanes_TestFatalTextColor,
        OutputPanes_TestDebugTextColor,

        /* Debugger Log Window */

        Debugger_LogWindow_LogInput,
        Debugger_LogWindow_LogStatus,
        Debugger_LogWindow_LogTime,

        /* Debugger Watch Item */

        Debugger_WatchItem_ValueNormal,
        Debugger_WatchItem_ValueInvalid,
        Debugger_WatchItem_ValueChanged,

        /* Qt Creator Color Tokens */

        Token_Basic_Black,
        Token_Basic_White,
        Token_Accent_Default,
        Token_Accent_Muted,
        Token_Accent_Subtle,
        Token_Background_Default,
        Token_Background_Muted,
        Token_Background_Subtle,
        Token_Foreground_Default,
        Token_Foreground_Muted,
        Token_Foreground_Subtle,
        Token_Text_Default,
        Token_Text_Muted,
        Token_Text_Subtle,
        Token_Text_Accent,
        Token_Stroke_Strong,
        Token_Stroke_Muted,
        Token_Stroke_Subtle,
        Token_Notification_Alert_Default,
        Token_Notification_Alert_Muted,
        Token_Notification_Alert_Subtle,
        Token_Notification_Success_Default,
        Token_Notification_Success_Muted,
        Token_Notification_Success_Subtle,
        Token_Notification_Neutral_Default,
        Token_Notification_Neutral_Muted,
        Token_Notification_Neutral_Subtle,
        Token_Notification_Danger_Default,
        Token_Notification_Danger_Muted,
        Token_Notification_Danger_Subtle,
        Token_Gradient01_Start,
        Token_Gradient01_End,
        Token_Gradient02_Start,
        Token_Gradient02_End,

        /* Timeline Library */
        Timeline_TextColor,
        Timeline_BackgroundColor1,
        Timeline_BackgroundColor2,
        Timeline_DividerColor,
        Timeline_HighlightColor,
        Timeline_PanelBackgroundColor,
        Timeline_PanelHeaderColor,
        Timeline_HandleColor,
        Timeline_RangeColor,

        /* VcsBase Plugin */
        VcsBase_FileStatusUnknown_TextColor,
        VcsBase_FileAdded_TextColor,
        VcsBase_FileModified_TextColor,
        VcsBase_FileDeleted_TextColor,
        VcsBase_FileRenamed_TextColor,
        VcsBase_FileUnmerged_TextColor,

        /* Bookmarks Plugin */
        Bookmarks_TextMarkColor,

        /* TextEditor Plugin */
        TextEditor_SearchResult_ScrollBarColor,
        TextEditor_Selection_ScrollBarColor,
        TextEditor_CurrentLine_ScrollBarColor,

        /* Debugger Plugin */
        Debugger_Breakpoint_TextMarkColor,

        /* ProjectExplorer Plugin */
        ProjectExplorer_TaskError_TextMarkColor,
        ProjectExplorer_TaskWarn_TextMarkColor,

        /* QmlDesigner Plugin */
        QmlDesigner_BackgroundColor,
        QmlDesigner_HighlightColor,
        QmlDesigner_FormEditorSelectionColor,
        QmlDesigner_FormEditorForegroundColor,
        QmlDesigner_BackgroundColorDarker,
        QmlDesigner_BackgroundColorDarkAlternate,
        QmlDesigner_TabLight,
        QmlDesigner_TabDark,
        QmlDesigner_ButtonColor,
        QmlDesigner_BorderColor,
        QmlDesigner_FormeditorBackgroundColor,
        QmlDesigner_AlternateBackgroundColor,
        QmlDesigner_ScrollBarHandleColor,

        /* Palette for DS Controls */
        DSpanelBackground,
        DSinteraction,
        DSerrorColor,
        DSwarningColor,
        DSdisabledColor,
        DSinteractionHover,
        DScontrolBackground,
        DScontrolBackgroundInteraction,
        DScontrolBackgroundDisabled,
        DScontrolBackgroundGlobalHover,
        DScontrolBackgroundHover,
        DScontrolOutline,
        DScontrolOutlineInteraction,
        DScontrolOutlineDisabled,
        DStextColor,
        DStextColorDisabled,
        DStextSelectionColor,
        DStextSelectedTextColor,

        /*New Creator Semantics for qtds*/

        DS_base_black,
        DS_base_white,

        DS_accent_default,
        DS_accent_muted,
        DS_accent_subtle,

        DS_background_default,
        DS_background_muted,
        DS_background_subtle,

        DS_forground_default,
        DS_foreground_muted,
        DS_foreground_subtle,

        DS_text_default,
        DS_text_muted,
        DS_text_subtle,
        DS_text_accent,

        DS_stroke_strong,
        DS_stroke_muted,
        DS_stroke_subtle,

        DS_notification_alert_default,
        DS_notification_alert_muted,
        DS_notification_alert_subtle,

        DS_notification_neutral_default,
        DS_notification_neutral_muted,
        DS_notification_neutral_subtle,

        DS_notification_danger_default,
        DS_notification_danger_muted,
        DS_notification_danger_subtle,

        DS_notification_success_default,
        DS_notification_success_muted,
        DS_notification_success_subtle,

        /*NEW for QtDS 4.0*/
        DScontrolBackground_toolbarIdle,
        DScontrolBackground_toolbarHover,
        DStoolbarBackground,
        DStoolbarIcon_blocked,
        DSthumbnailBackground_baseState,
        DSidleGreen,
        DSrunningGreen,
        DSstatusbarBackground,
        DScontrolBackground_statusbarIdle,
        DScontrolBackground_topToolbarHover,
        DSControlBackground_statusbarHover,
        DScontrolOutline_topToolbarIdle,
        DScontrolOutline_topToolbarHover,
        DSprimaryButton_hoverHighlight,
        DSstateBackgroundColor_hover,
        DSstateControlBackgroundColor_hover,
        DSstateControlBackgroundColor_globalHover,
        DSplaceholderTextColor,
        DSplaceholderTextColorInteraction,
        DSpopoutBackground,
        DSpopoutControlBackground_idle,
        DSpopoutControlBackground_hover,
        DSpopoutControlBackground_globalHover,
        DSpopoutControlBackground_interaction,
        DSpopoutControlBackground_disabled,
        DSpopoutPopupBackground,
        DSpopoutControlBorder_idle,
        DSpopoutControlBorder_hover,
        DSpopoutControlBorder_interaction,
        DSpopoutControlBorder_disabled,
        DSpopoutButtonBackground_idle,
        DSpopoutButtonBackground_hover,
        DSpopoutButtonBackground_interaction,
        DSpopoutButtonBackground_disabled,
        DSpopoutButtonBorder_idle,
        DSpopoutButtonBorder_hover,
        DSpopoutButtonBorder_interaction,
        DSpopoutButtonBorder_disabled,
        DSscrollBarTrack,
        DSscrollBarHandle,
        DSscrollBarHandle_idle,
        DSconnectionCodeEditor,
        DSpillText,
        DSpillTextSelected,
        DspillTextEdit,
        DSpillDefaultBackgroundIdle,
        DSpillDefaultBackgroundHover,
        DSpillOperatorBackgroundIdle,
        DSpillOperatorBackgroundHover,
        DSpillLiteralBackgroundIdle,
        DSpillLiteralBackgroundHover,
        DSconnectionEditorMicroToolbar,
        DSconnectionEditorButtonBackground_hover,
        DSconnectionEditorButtonBorder_hover,

        /*Legacy QtDS*/
        DSiconColor,
        DSiconColorHover,
        DSiconColorInteraction,
        DSiconColorDisabled,
        DSiconColorSelected,
        DSlinkIndicatorColor,
        DSlinkIndicatorColorHover,
        DSlinkIndicatorColorInteraction,
        DSlinkIndicatorColorDisabled,
        DSpopupBackground,
        DSpopupOverlayColor,
        DSsliderActiveTrack,
        DSsliderActiveTrackHover,
        DSsliderActiveTrackFocus,
        DSsliderInactiveTrack,
        DSsliderInactiveTrackHover,
        DSsliderInactiveTrackFocus,
        DSsliderHandle,
        DSsliderHandleHover,
        DSsliderHandleFocus,
        DSsliderHandleInteraction,
        DSsectionHeadBackground,
        DSstateDefaultHighlight,
        DSstateSeparatorColor,
        DSstateBackgroundColor,
        DSstatePreviewOutline,
        DSchangedStateText,
        DS3DAxisXColor,
        DS3DAxisYColor,
        DS3DAxisZColor,
        DSactionBinding,
        DSactionAlias,
        DSactionKeyframe,
        DSactionJIT,

        DStableHeaderBackground,
        DStableCellCurrent,
        DStableHeaderText,

        DSdockContainerBackground,
        DSdockContainerSplitter,
        DSdockAreaBackground,

        DSdockWidgetBackground,
        DSdockWidgetSplitter,
        DSdockWidgetTitleBar,

        DStitleBarText,
        DStitleBarIcon,
        DStitleBarButtonHover,
        DStitleBarButtonPress,

        DStabContainerBackground,
        DStabSplitter,

        DStabInactiveBackground,
        DStabInactiveText,
        DStabInactiveIcon,
        DStabInactiveButtonHover,
        DStabInactiveButtonPress,

        DStabActiveBackground,
        DStabActiveText,
        DStabActiveIcon,
        DStabActiveButtonHover,
        DStabActiveButtonPress,

        DStabFocusBackground,
        DStabFocusText,
        DStabFocusIcon,
        DStabFocusButtonHover,
        DStabFocusButtonPress,

        DSnavigatorBranch,
        DSnavigatorBranchIndicator,
        DSnavigatorItemBackground,
        DSnavigatorItemBackgroundHover,
        DSnavigatorItemBackgroundSelected,
        DSnavigatorText,
        DSnavigatorTextHover,
        DSnavigatorTextSelected,
        DSnavigatorIcon,
        DSnavigatorIconHover,
        DSnavigatorIconSelected,
        DSnavigatorAliasIconChecked,
        DSnavigatorDropIndicatorBackground,
        DSnavigatorDropIndicatorOutline,

        DSheaderViewBackground,
        DStableViewAlternateBackground,

        DStoolTipBackground,
        DStoolTipOutline,
        DStoolTipText,

        DSBackgroundColorNormal,
        DSBackgroundColorAlternate,

        DSUnimportedModuleColor,

        DSwelcomeScreenBackground,
        DSsubPanelBackground,
        DSthumbnailBackground,
        DSthumbnailLabelBackground,

        DSgreenLight,
        DSamberLight,
        DSredLight,

        DSstatePanelBackground,
        DSstateHighlight,

        TerminalForeground,
        TerminalBackground,
        TerminalSelection,
        TerminalFindMatch,

        TerminalAnsi0,
        TerminalAnsi1,
        TerminalAnsi2,
        TerminalAnsi3,
        TerminalAnsi4,
        TerminalAnsi5,
        TerminalAnsi6,
        TerminalAnsi7,
        TerminalAnsi8,
        TerminalAnsi9,
        TerminalAnsi10,
        TerminalAnsi11,
        TerminalAnsi12,
        TerminalAnsi13,
        TerminalAnsi14,
        TerminalAnsi15,
    };

    enum ImageFile {
        IconOverlayCSource,
        IconOverlayCppHeader,
        IconOverlayCppSource,
        IconOverlayPri,
        IconOverlayPrf,
        IconOverlayPro,
        StandardPixmapFileIcon,
        StandardPixmapDirIcon
    };

    enum Flag {
        DrawTargetSelectorBottom,
        DrawSearchResultWidgetFrame,
        DrawIndicatorBranch,
        DrawToolBarHighlights,
        DrawToolBarBorders,
        ComboBoxDrawTextShadow,
        DerivePaletteFromTheme,
        DerivePaletteFromThemeIfNeeded,
        FlatToolBars,
        FlatSideBarIcons,
        FlatProjectsMode,
        FlatMenuBar,
        ToolBarIconShadow,
        WindowColorAsBase,
        DarkUserInterface,
        QDSTheme
    };

    enum TokenFlag {
        UsedInToolbar = 1,
    };

    Q_ENUM(Color)
    Q_ENUM(ImageFile)
    Q_ENUM(Flag)
    Q_DECLARE_FLAGS(TokenFlags, TokenFlag)

    Q_INVOKABLE bool flag(Utils::Theme::Flag f) const;
    Q_INVOKABLE QColor color(Utils::Theme::Color role) const;
    QString imageFile(ImageFile imageFile, const QString &fallBack) const;
    QPalette palette() const;
    QStringList preferredStyles() const;
    QString defaultTextEditorColorScheme() const;
    StyleHelper::ToolbarStyle defaultToolbarStyle() const;

    QString id() const;
    QString filePath() const;
    QString displayName() const;
    void setDisplayName(const QString &displayName);

    void readSettings(QSettings &settings);

    Qt::ColorScheme colorScheme() const;
    static Qt::ColorScheme systemColorScheme();
    static QPalette initialPalette();

    static void setInitialPalette(Theme *initTheme);

    static void setHelpMenu(QMenu *menu);

    static Result<Color> colorToken(const QString &token, TokenFlags flags = {});
    static Color highlightFor(Color role);

protected:
    Theme(Theme *originTheme, QObject *parent = nullptr);
    ThemePrivate *d;

private:
    void readSettingsInternal(QSettings &settings);
    friend UTILS_EXPORT Theme *creatorTheme();
    QColor readNamedColorNoWarning(const QString &color) const;
    QPair<QColor, QString> readNamedColor(const QString &color) const;
};

UTILS_EXPORT Theme *creatorTheme();
UTILS_EXPORT QColor creatorColor(Theme::Color role);

} // namespace Utils
