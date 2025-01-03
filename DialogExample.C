/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "DialogExample.h"
#include "CsvUtil.h"
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTableView.h>
#include <Wt/WItemDelegate.h>
#include <Wt/Chart/WCartesianChart.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <fstream>
#include <iostream>

using namespace Wt;
using namespace std::string_literals;

class NumericItem : public WStandardItem
{
    public:
        virtual std::unique_ptr<WStandardItem> clone() const override
        {
            return std::make_unique<NumericItem>();
        }

        virtual void setData(const cpp17::any &data, ItemDataRole role = ItemDataRole::User) override
        {
            cpp17::any dt;

            if (role == ItemDataRole::Edit)
            {
                std::string s = asString(data).toUTF8();
                char *endptr;
                double d = strtod(s.c_str(), &endptr);
                if (*endptr == 0)
                  dt = cpp17::any(d);
                else
                  dt = data;
            }

            WStandardItem::setData(data, role);
        }
  };

/*
   * Reads a CSV file as an (editable) standard item model.
   */
std::shared_ptr<WAbstractItemModel> readCsvFile(const std::string &fname,
                                  WContainerWidget *parent)
{
    std::shared_ptr<WStandardItemModel> model = std::make_shared<WStandardItemModel>(0, 0);
    std::unique_ptr<NumericItem> prototype = std::make_unique<NumericItem>();
    model->setItemPrototype(std::move(prototype));
    std::ifstream f(fname.c_str());

    if (f)
    {
      readFromCsv(f, model.get());

      for (int row = 0; row < model->rowCount(); ++row)
      {
          for (int col = 0; col < model->columnCount(); ++col)
          {
              model->item(row, col)->setFlags(ItemFlag::Selectable | ItemFlag::Editable);
          }
      }

      return model;
    }
    else
    {
        WString error(WString::tr("error-missing-data"));
        error.arg(fname, CharEncoding::UTF8);
        parent->addWidget(std::make_unique<WText>(error));
        return 0;
    }
}


CategoryExample::CategoryExample():
  WContainerWidget()
{
    this->addWidget(std::make_unique<WText>(WString::tr("Category chart")));

    std::shared_ptr<WAbstractItemModel> model = readCsvFile(WApplication::appRoot() + "category.csv", this);

    if (!model)
      return;

    // Show a view that allows editing of the model.
    auto *w = this->addWidget(std::make_unique<WContainerWidget>());

    /*
    * Create the category chart.
    */
    auto *chart = this->addWidget(std::make_unique<Wt::Chart::WCartesianChart>());
    chart->setModel(model);        // set the model
    chart->setXSeriesColumn(0);    // set the column that holds the categories
    chart->setLegendEnabled(true); // enable the legend
    chart->setZoomEnabled(true);
    chart->setPanEnabled(true);

    // Automatically layout chart (space for axes, legend, ...)
    chart->setAutoLayoutEnabled(true);

    chart->setBackground(WColor(200,200,200));

    /*
    * Add all (but first) column as bar series
    */
    for (int i = 1; i < model->columnCount(); ++i)
    {
        std::unique_ptr<Wt::Chart::WDataSeries> s = std::make_unique<Wt::Chart::WDataSeries>(i, Wt::Chart::SeriesType::Bar);
        s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
        chart->addSeries(std::move(s));
    }

    auto &xAxis = chart->axis(Wt::Chart::Axis::X);
    xAxis.setTitle(Wt::WString::tr("Age category"));
    auto &yAxis = chart->axis(Wt::Chart::Axis::Y);
    yAxis.setTitle(Wt::WString::tr("Number of persons"));

    chart->resize(800, 400);

    chart->setMargin(30, Side::Top | Side::Bottom | Side::Left | Side::Right);
    chart->setPlotAreaPadding(80, Wt::Side::Top | Wt::Side::Bottom);
    chart->setPlotAreaPadding(140, Wt::Side::Left);

}


DialogExample::DialogExample(const WEnvironment& env)
  : WApplication(env),
    messageBox_(nullptr)
{
  messageResourceBundle().use(appRoot() + "dialogweb");
  Wt::WApplication::instance()->setLocale(Wt::WLocale("nl_NL"));

  setTitle(Wt::WString::tr("Dialog example"));

  WContainerWidget *textdiv = root()->addWidget(std::make_unique<WContainerWidget>());
  textdiv->setStyleClass("text");

  textdiv->addWidget(std::make_unique<WText>("<h2>"s + Wt::WString::tr("Wt dialogs example") + "</h2>"));
  textdiv->addWidget(std::make_unique<WText>( Wt::WString::tr("UseWMessageBox") + " <br />"));

  WContainerWidget *buttons = root()->addWidget(std::make_unique<WContainerWidget>());
  buttons->setStyleClass("buttons");

  WPushButton *button = buttons->addWidget(std::make_unique<WPushButton>(Wt::WString::tr("To English")));
  button->clicked().connect(this, &DialogExample::messageBox1);

  button = buttons->addWidget(std::make_unique<WPushButton>(Wt::WString::tr("To Dutch")));
  button->clicked().connect(this, &DialogExample::messageBox2);

  button = buttons->addWidget(std::make_unique<WPushButton>(Wt::WString::tr("Havoc!")));
  button->clicked().connect(this, &DialogExample::messageBox3);

  button = buttons->addWidget(std::make_unique<WPushButton>(Wt::WString::tr("Discard")));
  button->clicked().connect(this, &DialogExample::messageBox4);

  button = buttons->addWidget(std::make_unique<WPushButton>(Wt::WString::tr("Familiar")));
  button->clicked().connect(this, &DialogExample::custom);

  textdiv = root()->addWidget(std::make_unique<WContainerWidget>());
  textdiv->setStyleClass("text");

  status_ = textdiv->addWidget(std::make_unique<WText>(Wt::WString::tr("Go ahead...")));

  styleSheet().addRule(".buttons",
                       "padding: 5px;");
  styleSheet().addRule(".buttons BUTTON",
                       "padding-left: 4px; padding-right: 4px;"
                       "margin-top: 4px; display: block");

  // avoid scrollbar problems
  styleSheet().addRule(".text", "padding: 4px 8px");
  styleSheet().addRule("body", "margin: 0px;");

  auto *chartdiv = root()->addWidget(std::make_unique<WContainerWidget>());
  chartdiv->addWidget(std::make_unique<CategoryExample>());
}

void DialogExample::messageBox1()
{
  WMessageBox::show(Wt::WString::tr("Information"),
                    "Enjoy displaying messages with a one-liner.", StandardButton::Ok);
  setStatus("Ok'ed");
  Wt::WApplication::instance()->setLocale(Wt::WLocale("en_GB"));
}

void DialogExample::messageBox2()
{
  messageBox_
    = std::make_unique<WMessageBox>("Question",
              "Are you getting comfortable ?",
            Icon::Question,
            StandardButton::Yes | StandardButton::No | StandardButton::Cancel);

  messageBox_
    ->buttonClicked().connect(this, &DialogExample::messageBoxDone);

  messageBox_->animateShow
    (WAnimation(AnimationEffect::Pop | AnimationEffect::Fade, TimingFunction::Linear, 250));
  Wt::WApplication::instance()->setLocale(Wt::WLocale("nl_NL"));
}

void DialogExample::messageBox3()
{
  StandardButton
    result = WMessageBox::show("Confirm", "About to wreak havoc... Continue ?",
                   StandardButton::Ok | StandardButton::Cancel,
                   WAnimation(AnimationEffect::SlideInFromTop));

  if (result == StandardButton::Ok)
    setStatus("Wreaking havoc.");
  else
    setStatus("Cancelled!");
}

void DialogExample::messageBox4()
{
  messageBox_
    = std::make_unique<WMessageBox>("Warning!",
              "Are you sure you want to continue?\n"
              "You have unsaved changes.",
              Icon::None, StandardButton::None);

  messageBox_->addButton("Discard Modifications", StandardButton::Ok);
  WPushButton *continueButton
    = messageBox_->addButton("Cancel", StandardButton::Cancel);
  messageBox_->setDefaultButton(continueButton);

  messageBox_
    ->buttonClicked().connect(this, &DialogExample::messageBoxDone);

  messageBox_->setOffsets(0, Side::Bottom);
  messageBox_->animateShow
    (WAnimation(AnimationEffect::SlideInFromBottom
        | AnimationEffect::Fade, TimingFunction::Linear, 250));
}

void DialogExample::messageBoxDone(StandardButton result)
{
  switch (result) {
  case StandardButton::Ok:
    setStatus(Wt::WString::tr("Approved"));
    break;
  case StandardButton::Cancel:
    setStatus(Wt::WString::tr("Cancelled!")); break;
  case StandardButton::Yes:
    setStatus(Wt::WString::tr("Me too!")); break;
  case StandardButton::No:
    setStatus(Wt::WString::tr("Me neither!")); break;
  default:
    setStatus(Wt::WString::tr("Unknown result?"));
  }

  messageBox_.reset();
}

void DialogExample::custom()
{
  WDialog dialog("Personalia");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);

  dialog.contents()->addWidget(std::make_unique<WText>("Enter your name: "));
  WLineEdit *edit = dialog.contents()->addWidget(std::make_unique<WLineEdit>());
  WPushButton *ok = dialog.footer()->addWidget(std::make_unique<WPushButton>("Ok"));
  ok->setDefault(true);

  edit->setFocus();
  ok->clicked().connect(&dialog, &WDialog::accept);

  if (dialog.exec() == DialogCode::Accepted) {
    setStatus("Welcome, " + edit->text());
  } else {
    setStatus("Oh nevermind!");
  }
}

void DialogExample::setStatus(const WString& result)
{
  status_->setText(result);
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<DialogExample>(env);
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

