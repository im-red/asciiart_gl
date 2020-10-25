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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ********************************************************************************/

#include "asciiartglwidget.h"

#include <QDebug>

#include <QFile>
#include <algorithm>

using namespace std;

ASCIIArtGLWidget::ASCIIArtGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{

}

static float vertices[] =
{
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
};

void ASCIIArtGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    QFile vertFile(":/vert.vert");
    vertFile.open(QIODevice::ReadOnly);

    QFile simpleFragFile(":/simple.frag");
    simpleFragFile.open(QIODevice::ReadOnly);

    QFile asciiFragFile(":/ascii.frag");
    asciiFragFile.open(QIODevice::ReadOnly);

    m_simpleProgram.reset(new QOpenGLShaderProgram);
    m_simpleProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertFile.readAll().data());
    m_simpleProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, simpleFragFile.readAll().data());
    m_simpleProgram->link();

    vertFile.seek(0);

    m_asciiProgram.reset(new QOpenGLShaderProgram);
    m_asciiProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertFile.readAll().data());
    m_asciiProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, asciiFragFile.readAll().data());
    m_asciiProgram->link();

    glGenTextures(1, &m_texture);
    glGenTextures(1, &m_asciiTexture);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void ASCIIArtGLWidget::paintGL()
{
    glViewport(m_imageX, m_imageY, m_imageWidth, m_imageHeight);

    if (m_textureNeedUpdate)
    {
        m_textureNeedUpdate = false;
        qDebug() << "updateTexture";

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     m_image.width(), m_image.height(),
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     m_image.constBits());
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, m_asciiTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     m_asciiImage.width(), m_asciiImage.height(),
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     m_asciiImage.constBits());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (m_asciiImage.format() != QImage::Format_Invalid)
    {
        qDebug() << "valid asciiImage";

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_asciiTexture);

        m_asciiProgram->bind();

        glUniform1i(glGetUniformLocation(m_asciiProgram->programId(), "asciiTexture"), 1);
        glUniform1f(glGetUniformLocation(m_asciiProgram->programId(), "charWidth"),
                    m_asciiImage.width() / 256.0f / m_imageWidth);
        glUniform1f(glGetUniformLocation(m_asciiProgram->programId(), "charHeight"),
                    m_asciiImage.height() * 1.0f / m_imageHeight);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(float) / 4);
        glBindVertexArray(0);

        m_asciiProgram->release();
    }
    else
    {
        qDebug() << "invalid asciiImage";

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        m_simpleProgram->bind();

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(float) / 4);
        glBindVertexArray(0);

        m_simpleProgram->release();
    }
}

void ASCIIArtGLWidget::resizeGL(int w, int h)
{
    double imageRatio = m_image.width() * 1.0 / m_image.height();
    double widgetRatio = w * 1.0 / h;
    if (imageRatio >= widgetRatio)
    {
        m_imageWidth = w;
        m_imageHeight = static_cast<int>(m_imageWidth / imageRatio);
        m_imageX = 0;
        m_imageY = (h - m_imageHeight) / 2;
    }
    else
    {
        m_imageHeight = h;
        m_imageWidth = static_cast<int>(m_imageHeight * imageRatio);
        m_imageY = 0;
        m_imageX = (w - m_imageWidth) / 2;
    }
}

void ASCIIArtGLWidget::setImage(const QImage &image)
{
    m_image = image.convertToFormat(QImage::Format_RGB888);
    m_textureNeedUpdate = true;
    resizeGL(width(), height());
    update();
}

void ASCIIArtGLWidget::setAsciiImage(const QImage &asciiImage)
{
    m_asciiImage = asciiImage.convertToFormat(QImage::Format_RGB888);
    m_textureNeedUpdate = true;
    resizeGL(width(), height());
    update();
}
