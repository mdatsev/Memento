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

#ifndef MPVFRAMEBUFFER_H
#define MPVFRAMEBUFFER_H

#include <QQuickFramebufferObject>

#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvRenderer;

class MpvFramebuffer : public QQuickFramebufferObject
{
    Q_OBJECT
    friend class MpvRenderer;

public:
    MpvFramebuffer(QQuickItem *parent = nullptr);
    ~MpvFramebuffer();

    Renderer *createRenderer() const override;

    static void onUpdate(MpvFramebuffer *ctx);

Q_SIGNALS:
    void requestUpdate();

private Q_SLOTS:
    void doUpdate();

private:
    mpv_handle *m_mpv;

    mpv_render_context *m_mpv_gl;
};

#endif // MPVFRAMEBUFFER_H
