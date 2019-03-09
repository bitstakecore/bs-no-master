// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "privacydialog.h"
#include "ui_privacydialog.h"

#include "addressbookpage.h"
#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "coincontroldialog.h"
#include "optionsmodel.h"
#include "sendcoinsentry.h"
#include "walletmodel.h"
#include "coincontrol.h"
#include "spork.h"
#include "askpassphrasedialog.h"

#include <QClipboard>
#include <QSettings>
#include <utilmoneystr.h>
#include <QtWidgets>

PrivacyDialog::PrivacyDialog(QWidget* parent) : QDialog(parent),
                                                          ui(new Ui::PrivacyDialog),
                                                          walletModel(0),
                                                          currentBalance(-1)
{
    nDisplayUnit = 0; // just make sure it's not unitialized
    ui->setupUi(this);

    // "Spending 999999 xBS ought to be enough for anybody." - Bill Gates, 2017
    ui->labelMintAmountValue->setValidator( new QIntValidator(0, 999999, this) );

    // Default texts for (mini-) coincontrol
    ui->labelCoinControlQuantity->setText (tr("Coins automatically selected"));
    ui->labelCoinControlAmount->setText (tr("Coins automatically selected"));

    // Sunken frame for minting messages
    ui->TEMintStatus->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ui->TEMintStatus->setLineWidth (2);
    ui->TEMintStatus->setMidLineWidth (2);
    ui->TEMintStatus->setPlainText(tr("Mint Status: Okay"));

    // Coin Control signals
    connect(ui->pushButtonCoinControl, SIGNAL(clicked()), this, SLOT(coinControlButtonClicked()));

    // Coin Control: clipboard actions
    QAction* clipboardQuantityAction = new QAction(tr("Copy quantity"), this);
    QAction* clipboardAmountAction = new QAction(tr("Copy amount"), this);
    connect(clipboardQuantityAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardQuantity()));
    connect(clipboardAmountAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAmount()));
    ui->labelCoinControlQuantity->addAction(clipboardQuantityAction);
    ui->labelCoinControlAmount->addAction(clipboardAmountAction);

    // Denomination labels
    ui->labelzDenom1Text->setText(tr("Denom. with value <b>1</b>:"));
    ui->labelzDenom2Text->setText(tr("Denom. with value <b>5</b>:"));
    ui->labelzDenom3Text->setText(tr("Denom. with value <b>10</b>:"));
    ui->labelzDenom4Text->setText(tr("Denom. with value <b>50</b>:"));
    ui->labelzDenom5Text->setText(tr("Denom. with value <b>100</b>:"));
    ui->labelzDenom6Text->setText(tr("Denom. with value <b>500</b>:"));
    ui->labelzDenom7Text->setText(tr("Denom. with value <b>1000</b>:"));
    ui->labelzDenom8Text->setText(tr("Denom. with value <b>5000</b>:"));

    // AutoMint status
    ui->label_AutoMintStatus->setText(tr("AutoMint Status:"));

    // Global Supply labels
    ui->labelZsupplyText1->setText(tr("Denom. <b>1</b>:"));
    ui->labelZsupplyText5->setText(tr("Denom. <b>5</b>:"));
    ui->labelZsupplyText10->setText(tr("Denom. <b>10</b>:"));
    ui->labelZsupplyText50->setText(tr("Denom. <b>50</b>:"));
    ui->labelZsupplyText100->setText(tr("Denom. <b>100</b>:"));
    ui->labelZsupplyText500->setText(tr("Denom. <b>500</b>:"));
    ui->labelZsupplyText1000->setText(tr("Denom. <b>1000</b>:"));
    ui->labelZsupplyText5000->setText(tr("Denom. <b>5000</b>:"));
    
    // BS settings
    QSettings settings;
    if (!settings.contains("nSecurityLevel")){
        nSecurityLevel = 42;
        settings.setValue("nSecurityLevel", nSecurityLevel);
    }
    else{
        nSecurityLevel = settings.value("nSecurityLevel").toInt();
    }

    if (!settings.contains("fMinimizeChange")){
        fMinimizeChange = false;
        settings.setValue("fMinimizeChange", fMinimizeChange);
    }
    else{
        fMinimizeChange = settings.value("fMinimizeChange").toBool();
    }
    ui->checkBoxMinimizeChange->setChecked(fMinimizeChange);

    // Start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);

    // Hide those placeholder elements needed for CoinControl interaction
    ui->WarningLabel->hide();    // Explanatory text visible in QT-Creator
    ui->dummyHideWidget->hide(); // Dummy widget with elements to hide

}

PrivacyDialog::~PrivacyDialog()
{
    delete ui;
}

void PrivacyDialog::setModel(WalletModel* walletModel)
{
    this->walletModel = walletModel;

    if (walletModel && walletModel->getOptionsModel()) {
        // Keep up to date with wallet
        setBalance(walletModel->getBalance(), walletModel->getUnconfirmedBalance(), walletModel->getImmatureBalance(),
                   0,0,0,
                   walletModel->getWatchBalance(), walletModel->getWatchUnconfirmedBalance(), walletModel->getWatchImmatureBalance());

        connect(walletModel, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this,
                               SLOT(setBalance(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));
        connect(walletModel->getOptionsModel(), SIGNAL(zeromintEnableChanged(bool)), this, SLOT(updateAutomintStatus()));
        connect(walletModel->getOptionsModel(), SIGNAL(zeromintPercentageChanged(int)), this, SLOT(updateAutomintStatus()));
        ui->securityLevel->setValue(nSecurityLevel);
    }
}

void PrivacyDialog::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void PrivacyDialog::on_addressBookButton_clicked()
{
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
    dlg.setModel(walletModel->getAddressTableModel());
    if (dlg.exec()) {
        ui->payTo->setText(dlg.getReturnValue());
    }
}


void PrivacyDialog::on_pushButtonMintReset_clicked()
{
}

void PrivacyDialog::on_pushButtonSpentReset_clicked()
{
}

static inline int64_t roundint64(double d)
{
    return (int64_t)(d > 0 ? d + 0.5 : d - 0.5);
}


void PrivacyDialog::on_payTo_textChanged(const QString& address)
{
    updateLabel(address);
}

// Coin Control: copy label "Quantity" to clipboard
void PrivacyDialog::coinControlClipboardQuantity()
{
    GUIUtil::setClipboard(ui->labelCoinControlQuantity->text());
}

// Coin Control: copy label "Amount" to clipboard
void PrivacyDialog::coinControlClipboardAmount()
{
    GUIUtil::setClipboard(ui->labelCoinControlAmount->text().left(ui->labelCoinControlAmount->text().indexOf(" ")));
}

// Coin Control: button inputs -> show actual coin control dialog
void PrivacyDialog::coinControlButtonClicked()
{
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    CoinControlDialog dlg;
    dlg.setModel(walletModel);
    dlg.exec();
    coinControlUpdateLabels();
}

// Coin Control: update labels
void PrivacyDialog::coinControlUpdateLabels()
{
    if (!walletModel || !walletModel->getOptionsModel() || !walletModel->getOptionsModel()->getCoinControlFeatures())
        return;

     // set pay amounts
    CoinControlDialog::payAmounts.clear();

    if (CoinControlDialog::coinControl->HasSelected()) {
        // Actual coin control calculation
        CoinControlDialog::updateLabels(walletModel, this);
    } else {
        ui->labelCoinControlQuantity->setText (tr("Coins automatically selected"));
        ui->labelCoinControlAmount->setText (tr("Coins automatically selected"));
    }
}

bool PrivacyDialog::updateLabel(const QString& address)
{
    if (!walletModel)
        return false;

    // Fill in label from address book, if address has an associated label
    QString associatedLabel = walletModel->getAddressTableModel()->labelForAddress(address);
    if (!associatedLabel.isEmpty()) {
        ui->addAsLabel->setText(associatedLabel);
        return true;
    }

    return false;
}

void PrivacyDialog::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                               const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance,
                               const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{

    currentBalance = balance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;

    CWalletDB walletdb(pwalletMain->strWalletFile);


    int nBestHeight = chainActive.Height();

    int64_t nCoins = 0;
    int64_t nSumPerCoin = 0;
    int64_t nUnconfirmed = 0;
    int64_t nImmature = 0;
    QString strDenomStats, strUnconfirmed = "";

    CAmount nLockedBalance = 0;
    if (walletModel) {
        nLockedBalance = walletModel->getLockedBalance();
    }


    // Display AutoMint status
    updateAutomintStatus();

    // Display global supply
}

void PrivacyDialog::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if (currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance,
                       0,0,0,
                       currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance);
    }
}

void PrivacyDialog::showOutOfSyncWarning(bool fShow)
{
}

void PrivacyDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Escape) // press esc -> ignore
    {
        this->QDialog::keyPressEvent(event);
    } else {
        event->ignore();
    }
}

void PrivacyDialog::updateAutomintStatus()
{
}
