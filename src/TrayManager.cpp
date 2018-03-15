#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QtGui/QDesktopServices>
#include "Settings.h"
#include "TrayManager.h"
#include "MainWidget.h"

#include "Autorun.h"

TrayManager &TrayManager::instance()
{
    static TrayManager _instance;
    return _instance;
}

TrayManager::TrayManager(QObject *parent)
    : QObject(parent)
{
}

void TrayManager::setupTray(MainWidget *parent)
{
    mainWidget = parent;
    if(!QSystemTrayIcon::isSystemTrayAvailable()){
        QMessageBox::critical(mainWidget,":(","Ninja Mode is not available on this computer. Try again later :P");
    }

    trayMenu = new QMenu();
    createActions(trayMenu);

    trayIcon = new QSystemTrayIcon(this);

    /*
    // Unbind "tray icon activates window"
     https://trello.com/c/qyCrTMfy/39-tray-kliknięcie-niech-zawsze-pokazuje-menu-otwieramy-przez-open-z-tego-menu

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    */
#ifndef __APPLE__
    trayIcon->setIcon(QIcon(MAIN_ICON));
#else
    QIcon macOSIcon(":/Icons/res/AppIcon_Dark.png");
    macOSIcon.setIsMask(true);
    trayIcon->setIcon(macOSIcon);
#endif
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

#ifdef __APPLE__
    widget = new Widget_M();
#endif
}

void TrayManager::setupSettings()
{
    qDebug() << "Setting up settings";
    // set checkboxes
    autoStartAct->setDisabled(false);
    autoStartAct->setChecked(Autorun::checkAutorun());
    trackerAct->setChecked(settings.value(SETT_TRACK_PC_ACTIVITIES, false).toBool());
    widgetAct->setChecked(settings.value(SETT_SHOW_WIDGET, false).toBool());

    // act on the saved settings
    this->autoStart(autoStartAct->isChecked());
    this->tracker(trackerAct->isChecked());
}

void TrayManager::updateStopMenu(bool canBeStopped, QString timerName)
{
    stopTaskAct->setText("Stop " + timerName);
    stopTaskAct->setEnabled(canBeStopped);
}

void TrayManager::autoStart(bool checked)
{
    if(checked){
        Autorun::enableAutorun();
    }else{
        Autorun::disableAutorun();
    }
}

void TrayManager::tracker(bool checked)
{
    settings.setValue(SETT_TRACK_PC_ACTIVITIES, checked);
    emit pcActivitiesValueChanged(checked);
}

void TrayManager::widgetToggl(bool checked)
{
    settings.setValue(SETT_SHOW_WIDGET, checked);
    if(checked){
        widget->showMe();
    }else{
        widget->hideMe();
    }
}

void TrayManager::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason != QSystemTrayIcon::Context){
        mainWidget->open();
    }
}

void TrayManager::openCloseWindowText(bool isBeingOpened) {
    if (!isBeingOpened) {                   // was active
        openAct->setText("Open window");    // and menu says "hey try to open it again"
    } else {
        openAct->setText("Close window");
    }
}

void TrayManager::openCloseWindowAction() {
    if (openAct->text() == "Open window") {
        mainWidget->open();
    } else {
        mainWidget->close();
    }
}

void TrayManager::contactSupport() {
    QUrl mail("mailto:support@timecamp.com");
    QDesktopServices::openUrl(mail);
};

void TrayManager::createActions(QMenu *menu)
{
    openAct = new QAction(tr("Open window"), this);
    openAct->setStatusTip(tr("Opens TimeCamp interface"));
//    connect(openAct, &QAction::triggered, mainWidget, &MainWidget::open);
    connect(openAct, &QAction::triggered, this, &TrayManager::openCloseWindowAction);

    startTaskAct = new QAction(tr("Start timer"), this);
    startTaskAct->setStatusTip(tr("Go to task selection screen"));
    startTaskAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_N);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    startTaskAct->setShortcutVisibleInContextMenu(true);
#endif
    startTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(startTaskAct, &QAction::triggered, mainWidget, &MainWidget::startTask);

    stopTaskAct = new QAction(tr("Stop timer"), this);
    stopTaskAct->setStatusTip(tr("Stop currently running timer"));
    stopTaskAct->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_M);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    stopTaskAct->setShortcutVisibleInContextMenu(true);
#endif
    stopTaskAct->setShortcutContext(Qt::ApplicationShortcut);
    connect(stopTaskAct, &QAction::triggered, mainWidget, &MainWidget::stopTask);

    trackerAct = new QAction(tr("Track computer activities"), this);
    trackerAct->setCheckable(true);
    connect(trackerAct, &QAction::triggered, this, &TrayManager::tracker);

    widgetAct = new QAction(tr("Toggle time widget"), this);
    widgetAct->setCheckable(true);
    connect(widgetAct, &QAction::triggered, this, &TrayManager::widgetToggl);

    autoStartAct = new QAction(tr("Start with computer"), this);
    autoStartAct->setDisabled(true); // disable by default, till we login
    autoStartAct->setCheckable(true);
    connect(autoStartAct, &QAction::triggered, this, &TrayManager::autoStart);

    helpAct = new QAction(tr("Contact support"), this);
    helpAct->setStatusTip(tr("Need help? Talk to one of our support gurus"));
    connect(helpAct, &QAction::triggered, this, &TrayManager::contactSupport);

    quitAct = new QAction(tr("Quit"), this);
    quitAct->setStatusTip(tr("Close the app"));
    connect(quitAct, &QAction::triggered, mainWidget, &MainWidget::quit);


    menu->addAction(openAct);
    menu->addSeparator();
    menu->addAction(startTaskAct);
    menu->addAction(stopTaskAct);
    menu->addSeparator();
    menu->addAction(trackerAct);
    menu->addAction(autoStartAct);
    menu->addAction(widgetAct);
    menu->addSeparator();
    menu->addAction(helpAct);
    menu->addSeparator();
    menu->addAction(quitAct);
}

void TrayManager::loginLogout(bool loggedIn, QString tooltipText)
{
    qDebug() << "Login/Logout action";
    startTaskAct->setEnabled(loggedIn);
    stopTaskAct->setEnabled(loggedIn);
    trackerAct->setEnabled(loggedIn);
    trayIcon->setToolTip(tooltipText);
    if (loggedIn) {
        this->setupSettings();
    } else {
        emit pcActivitiesValueChanged(false); // don't track PC activities
    }
}
