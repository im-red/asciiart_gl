/*********************************************************************************
 * MIT License
 *
 * Copyright (c) 2020 Jia Lihong
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ********************************************************************************/

#include "mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPicture>
#include <QPixmap>
#include <map>
#include <vector>

#include "ui_mainwindow.h"
#include "util.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  util::setTipFunction(
      [this](const QString &s) { statusBar()->showMessage(s, 0); });

  on_allButton_clicked();
}

MainWindow::~MainWindow() { delete ui; }

QImage MainWindow::generateAsciiImage() {
  QFont f = ui->fontComboBox->currentFont();
  f.setPointSize(ui->spinBox->value());

  set<QChar> v;
  const QString &text = ui->plainTextEdit->toPlainText();
  for (const auto &c : text) {
    v.insert(c);
  }
  v.insert(' ');
  return generateAsciiImage(v, f);
}

QImage MainWindow::generateAsciiImage(const std::set<QChar> &chars,
                                      const QFont &font) {
  QFontMetrics fm(font);

  // QVector/QMap is convenient to print with qDebug
  QVector<QChar> asciiTable(256, ' ');
  QMap<int, QChar> gray2char;

  int width = fm.averageCharWidth();
  int height = fm.height();

  QImage tmp(width, height, QImage::Format_Grayscale8);
  for (auto c : chars) {
    tmp.fill(Qt::white);
    QPainter painter(&tmp);
    painter.setFont(font);
    painter.drawText(0, 0, width, height, Qt::AlignCenter, c);
    int whiteTotal = 0;
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        whiteTotal += tmp.pixelColor(x, y).red();
      }
    }
    int index =
        static_cast<int>(whiteTotal * 1.0 / (255 * width * height) * 255);
    gray2char[index] = c;
  }

  const int minGray = gray2char.firstKey();
  const int maxGray = gray2char.lastKey();
  const int range = maxGray - minGray;
  if (range == 0) {
    ui->grayMapTextEdit->setPlainText("");
    return QImage();
  }

  vector<pair<int, QChar>> pv;

  for (auto iter = gray2char.begin(); iter != gray2char.end(); iter++) {
    int gray = iter.key();
    QChar c = iter.value();
    int index = static_cast<int>((gray - minGray) * 1.0 / range * 255);
    asciiTable[index] = c;
    pv.push_back(pair<int, QChar>(index, c));
  }

  for (int i = 0; i < pv.size() - 1; i++) {
    auto p0 = pv[i];
    auto p1 = pv[i + 1];
    int index0 = p0.first;
    int index1 = p1.first;
    QChar c0 = p0.second;
    QChar c1 = p1.second;
    for (int j = index0 + 1; j < (index0 + index1) / 2; j++) {
      asciiTable[j] = c0;
    }
    for (int j = (index0 + index1) / 2; j < index1; j++) {
      asciiTable[j] = c1;
    }
  }

  QImage result(256 * width, height, QImage::Format_RGB888);
  result.fill(Qt::white);
  QPainter painter(&result);
  painter.setFont(font);
  for (int i = 0; i < 256; i++) {
    int x = width * i;
    painter.drawText(x, 0, width, height, Qt::AlignCenter, asciiTable[i]);
  }

  QString s;
  for (int i = 0; i < asciiTable.size(); i++) {
    s += QString("%1: %2\n").arg(i).arg(asciiTable[i]);
  }
  ui->grayMapTextEdit->setPlainText(s);

  return result;
}

void MainWindow::on_allButton_clicked() {
  QString s;
  for (char c = 32; c < 127; c++) {
    s += c;
  }
  ui->plainTextEdit->setPlainText(s);
}

void MainWindow::on_numberButton_clicked() {
  QString s;
  for (char c = '0'; c <= '9'; c++) {
    s += c;
  }
  ui->plainTextEdit->setPlainText(s);
}

void MainWindow::on_letterButton_clicked() {
  QString s;
  for (char c = 'a'; c <= 'z'; c++) {
    s += c;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    s += c;
  }
  ui->plainTextEdit->setPlainText(s);
}

void MainWindow::on_okButton_clicked() {
  ui->openGLWidget->setImage(m_image.mirrored());
  QImage asciiImage = generateAsciiImage();
  ui->openGLWidget->setAsciiImage(asciiImage.mirrored());
}

void MainWindow::on_openButton_clicked() {
  QString path = QFileDialog::getOpenFileName();
  if (path == "") {
    return;
  }

  ui->fileLineEdit->setText(path);
  m_image = util::loadImage(path);
  ui->okButton->setEnabled(m_image.format() != QImage::Format_Invalid);
}
