/*
 * Copyright (c) 2010 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include "QtContactEditWindow.h"

#include <algorithm>

#include <QScrollArea>
#include <QBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "Swift/QtUI/QtSwiftUtil.h"
#include "QtContactEditWidget.h"

namespace Swift {

QtContactEditWindow::QtContactEditWindow() : contactEditWidget_(NULL) {
	resize(300,300);
	setWindowTitle("Edit contact");
	setContentsMargins(0,0,0,0);

	QBoxLayout* layout = new QVBoxLayout(this);

	jidLabel_ = new QLabel(this);
	jidLabel_->setAlignment(Qt::AlignHCenter);
	layout->addWidget(jidLabel_);

	groupsLayout_ = new QVBoxLayout();
	groupsLayout_->setContentsMargins(0,0,0,0);
	layout->addLayout(groupsLayout_);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	layout->addLayout(buttonLayout);
	QPushButton* removeButton = new QPushButton("Remove contact", this);
	connect(removeButton, SIGNAL(clicked()), this, SLOT(handleRemoveContact()));
	buttonLayout->addWidget(removeButton);
	QPushButton* okButton = new QPushButton("Ok", this);
	connect(okButton, SIGNAL(clicked()), this, SLOT(handleUpdateContact()));
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
}

void QtContactEditWindow::setContact(const JID& jid, const std::string& name, const std::vector<std::string>& groups, const std::set<std::string>& allGroups) {
	delete contactEditWidget_;
	jid_ = jid;
	jidLabel_->setText("<b>" + P2QSTRING(jid.toString()) + "</b>");

	contactEditWidget_ = new QtContactEditWidget(allGroups, this);
	groupsLayout_->addWidget(contactEditWidget_);
	contactEditWidget_->setName(name);
	contactEditWidget_->setSelectedGroups(groups);
}

void QtContactEditWindow::setEnabled(bool b) {
	QWidget::setEnabled(b);
}

void QtContactEditWindow::show() {
	QWidget::show();
	QWidget::activateWindow();
}

void QtContactEditWindow::hide() {
	QWidget::hide();
}

void QtContactEditWindow::handleRemoveContact() {
	QMessageBox msgBox;
	msgBox.setWindowTitle("Confirm contact deletion");
	msgBox.setText("Are you sure you want to delete this contact?");
	msgBox.setInformativeText(QString("This will remove the contact '%1' from all groups they may be in.").arg(P2QSTRING(jid_.toString())));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	int ret = msgBox.exec();
	if (ret == QMessageBox::Yes) {
		onRemoveContactRequest();
	}
}

void QtContactEditWindow::handleUpdateContact() {
	onChangeContactRequest(contactEditWidget_->getName(), contactEditWidget_->getSelectedGroups());
}

}