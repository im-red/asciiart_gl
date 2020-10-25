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

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_5_Core>

#include <memory>

class ASCIIArtGLWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
public:
    ASCIIArtGLWidget(QWidget *parent = nullptr);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void setImage(const QImage &image);
    void setAsciiImage(const QImage &asciiImage);

private:
    QImage m_image;
    QImage m_asciiImage;

    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageX = 0;
    int m_imageY = 0;

    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    unsigned m_texture = 0;
    unsigned m_asciiTexture = 0;
    bool m_textureNeedUpdate = false;
    std::unique_ptr<QOpenGLShaderProgram> m_asciiProgram;
    std::unique_ptr<QOpenGLShaderProgram> m_simpleProgram;
};

