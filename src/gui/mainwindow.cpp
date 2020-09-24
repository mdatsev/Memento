#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mpvadapter.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    m_player = new MpvAdapter(m_ui->m_mpv, this);

    // Toolbar Actions
    connect(m_ui->m_actionOpen, &QAction::triggered, m_player, &PlayerAdapter::open);

    // Buttons
    connect(m_ui->m_controls, &PlayerControls::play, m_player, &PlayerAdapter::play);
    connect(m_ui->m_controls, &PlayerControls::pause, m_player, &PlayerAdapter::pause);
    connect(m_ui->m_controls, &PlayerControls::seekForward, m_player, &PlayerAdapter::seekForward);
    connect(m_ui->m_controls, &PlayerControls::seekBackward, m_player, &PlayerAdapter::seekBackward);

    // Slider
    connect(m_ui->m_controls, &PlayerControls::sliderMoved, m_player, &PlayerAdapter::seek);
    connect(m_player, &PlayerAdapter::durationChanged, m_ui->m_controls, &PlayerControls::setDuration);
    connect(m_player, &PlayerAdapter::positionChanged, m_ui->m_controls, &PlayerControls::setPosition);

    // State changes
    connect(m_player, &PlayerAdapter::stateChanged, m_ui->m_controls, &PlayerControls::setPaused);
}

MainWindow::~MainWindow() {
    delete m_ui;
    delete m_player;
}