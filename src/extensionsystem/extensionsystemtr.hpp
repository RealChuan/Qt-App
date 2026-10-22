// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <QCoreApplication>

namespace ExtensionSystem {

struct Tr
{
    // 上下文必须与命名空间一致，否则 lupdate/翻译查找会对不上
    Q_DECLARE_TR_FUNCTIONS(ExtensionSystem)
};

} // namespace ExtensionSystem
