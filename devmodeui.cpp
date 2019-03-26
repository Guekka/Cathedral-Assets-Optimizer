#include "devmodeui.h"
#include "ui_devmodeui.h"
#include "Optimiser.hpp"

//UI used to test some functions. Will be removed once the concerned functions will be 100% working. Debug log will probably be kept.
//Once this will be removed, public Optimiser functions will be able to be switched to private


devModeUI::devModeUI(Optimiser *optimiser) :
    ui(new Ui::devModeUI)
{
    ui->setupUi(this);

    optimiser->loadSettings();

    connect(ui->CreateBSA, &QPushButton::clicked, this, [=]()
    {
        optimiser->bsaCreate();
    });

    connect(ui->ExtractBSA, &QPushButton::clicked, this, [=]()
    {
        //optimiser->bsaExtract();
    });

    connect(ui->Setup, &QPushButton::clicked, this, [=]()
    {
        optimiser->setup();
    });

    connect(ui->MoveAssets, &QPushButton::clicked, this, [=]()
    {
        optimiser->splitAssets();
    });
}

devModeUI::~devModeUI()
{
    delete ui;
}