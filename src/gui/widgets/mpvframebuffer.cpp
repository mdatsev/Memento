////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2021 Ripose
//
// This file is part of Memento.
//
// Memento is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License.
//
// Memento is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Memento.  If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include "mpvframebuffer.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>

#include "../../util/globalmediator.h"
#include "mpvwidget.h"

static void on_mpv_redraw(void *ctx)
{
    MpvFramebuffer::onUpdate((MpvFramebuffer *)ctx);
}

static void *get_proc_address_mpv(void *ctx, const char *name)
{
    Q_UNUSED(ctx)

    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
    {
        return nullptr;
    }

    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

class MpvRenderer : public QQuickFramebufferObject::Renderer
{
public:
    MpvRenderer(MpvFramebuffer *parent) : m_parent(parent) {};

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    void render() override;

private:
    MpvFramebuffer *m_parent;
};

QOpenGLFramebufferObject *MpvRenderer::createFramebufferObject(const QSize &size)
{
    if (m_parent->m_mpv_gl == nullptr)
    {
        mpv_opengl_init_params gl_init_params{
            get_proc_address_mpv,
            nullptr,
            nullptr
        };
        mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };

        if (mpv_render_context_create(&m_parent->m_mpv_gl, m_parent->m_mpv, params) < 0)
        {
            Q_EMIT GlobalMediator::getGlobalMediator()->showCritical(
                "Could not start mpv",
                "MpvWidget: Failed to initialize mpv GL context"
            );
            QCoreApplication::exit(EXIT_FAILURE);
        }
        mpv_render_context_set_update_callback(m_parent->m_mpv_gl, on_mpv_redraw, m_parent);
    }

    return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
}

void MpvRenderer::render()
{
    m_parent->window()->resetOpenGLState();

    QOpenGLFramebufferObject *fbo = framebufferObject();
    mpv_opengl_fbo mpfbo {
        .fbo = static_cast<int>(fbo->handle()),
        .w = fbo->width(),
        .h = fbo->height(),
        .internal_format = 0
    };
    int flip_y{0};

    mpv_render_param params[] = {
        // Specify the default framebuffer (0) as target. This will
        // render onto the entire screen. If you want to show the video
        // in a smaller rectangle or apply fancy transformations, you'll
        // need to render into a separate FBO and draw it manually.
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        // Flip rendering (needed due to flipped GL coordinate system).
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(m_parent->m_mpv_gl, params);

    m_parent->window()->resetOpenGLState();
}

MpvFramebuffer::MpvFramebuffer(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_mpv(nullptr),
      m_mpv_gl(nullptr)
{
    MpvWidget *widget =
        (MpvWidget *)GlobalMediator::getGlobalMediator()->getPlayerWidget();
    m_mpv = widget->getHandle();
    connect(
        this, &MpvFramebuffer::requestUpdate,
        this, &MpvFramebuffer::doUpdate
    );
}

MpvFramebuffer::~MpvFramebuffer()
{
    if (m_mpv_gl)
    {
        mpv_render_context_free(m_mpv_gl);
    }
    mpv_terminate_destroy(m_mpv);
}

void MpvFramebuffer::onUpdate(MpvFramebuffer *ctx)
{
    Q_EMIT ctx->requestUpdate();
}

void MpvFramebuffer::doUpdate()
{
    update();
}

QQuickFramebufferObject::Renderer *MpvFramebuffer::createRenderer() const
{
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer((MpvFramebuffer *)this);
}
