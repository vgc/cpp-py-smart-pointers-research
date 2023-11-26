#!/usr/bin/python3

import gc
import sys
import unittest
from x06 import Action, Widget

def changeName(x):
    x.name = "newName"

def pyRefcount(obj):
    return sys.getrefcount(obj) - 2

class Tests(unittest.TestCase):

    def testAction(self):
        action = Action()
        self.assertEqual(action.name, "")
        action.name = "myAction"
        self.assertEqual(action.name, "myAction")
        action.setCallback(lambda x = action : changeName(x))
        action.executeCallback();
        self.assertEqual(action.name, "newName")

    def testWidget(self):
        action = Action()
        action.name = "myAction"
        action.setCallback(lambda x = action : changeName(x))

        widget = Widget()
        widget.action = action
        self.assertEqual(widget.action, action)
        widget.triggerAction()
        self.assertEqual(action.name, "newName")

    def testActionWeakPtr(self):
        actionShared = Action()
        actionShared2 = actionShared
        action = actionShared.toWeak()
        action2 = actionShared.toWeak()
        self.assertEqual(action.name, "")
        action.name = "myAction"
        self.assertEqual(action.name, "myAction")
        self.assertEqual(action2.name, "myAction")
        self.assertEqual(actionShared.name, "myAction")
        self.assertEqual(actionShared2.name, "myAction")
        action.setCallback(lambda x = action : changeName(x))
        action.executeCallback();
        self.assertEqual(action.name, "newName")

    def testWidgetWeakPtr(self):
        action = Action()
        action.name = "myAction"
        action.setCallback(lambda x = action : changeName(x))

        widgetShared = Widget()
        widget = widgetShared.toWeak()
        widget.action = action
        self.assertEqual(widget.action, action)
        widget.triggerAction()
        self.assertEqual(action.name, "newName")

        self.assertEqual(widget, widget.toShared())
        self.assertEqual(widgetShared, widgetShared.toWeak())
        self.assertEqual(widget, widget.toShared().toWeak())


    def testWidgetRefCounter(self):
        widget = Widget()
        refCounter = widget.refCounter()
        self.assertEqual(pyRefcount(widget), 1)
        self.assertEqual(refCounter.count, 1)

        widgetAlias = widget # Two Python variables to the same C++ std::shared_ptr
        self.assertEqual(pyRefcount(widget), 2)
        self.assertEqual(refCounter.count, 1)

        # Test removing the alias
        del widgetAlias
        self.assertEqual(pyRefcount(widget), 1)
        self.assertEqual(refCounter.count, 1)

        # Test removing the last reference to widget
        #
        # Note that we cannot test sys.getrefcount(widget) == 0, since there is
        # no widget anymore
        #
        # Also, the refCounter.count == 0 test assumes that the widget was
        # garbage collected right after `del widget`, which is normally the
        # case since there was no cycle, so it could directly see that the
        # refcount went to 0.
        #
        del widget
        self.assertEqual(refCounter.count, 0)

    def testWeakDoesntIncreaseRefCount(self):
        action = Action() # shared
        actionRefCounter = action.refCounter()
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(actionRefCounter.count, 1) # pybind11 holder for `action`

        widget = Widget()
        widget.action = action
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(actionRefCounter.count, 2) # pybind11 holder for `action`+ `widget.action_`

        action2 = widget.action # weak
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(actionRefCounter.count, 2) # pybind11 holder for `action`+ `widget.action_`

    def testConversionBetweenWeakAndShared(self):
        action = Action() # shared
        action2 = action.toWeak()

        actionRefCounter = action.refCounter()
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(actionRefCounter.count, 1) # pybind11 holder for `action`

        widget = Widget()
        widget.action = action
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(actionRefCounter.count, 2) # pybind11 holder for `action`+ `widget.action_`

        action2 = widget.action # weak
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(actionRefCounter.count, 2) # pybind11 holder for `action`+ `widget.action_`

    def testEqualityBetweenWeakAndShared(self):
        action = Action() # shared
        widget = Widget()
        widget.action = action
        action2 = widget.action # weak
        self.assertEqual(action2, action)
        self.assertEqual(action, action2)

    def testMemoryLeak(self):
        widget = Widget()
        widget.name = "myWidget"
        widgetRefCounter = widget.refCounter()
        self.assertEqual(pyRefcount(widget), 1)
        self.assertEqual(widgetRefCounter.count, 1)

        action = Action()
        action.name = "myAction"
        action.setCallback(lambda x = widget : changeName(x))
        actionRefCounter = action.refCounter()
        self.assertEqual(pyRefcount(widget), 2)     # `widget` + lambda
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(widgetRefCounter.count, 1) # pybind11 holder for `widget`
        self.assertEqual(actionRefCounter.count, 1) # pybind11 holder for `action`

        widget.action = action
        self.assertEqual(pyRefcount(widget), 2)     # `widget` + lambda
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(widgetRefCounter.count, 1) # pybind11 holder for `widget`
        self.assertEqual(actionRefCounter.count, 2) # pybind11 holder for `action` + `widget.action_`

        widget.triggerAction()
        self.assertEqual(widget.name, "newName")

        del action
        del widget
        gc.collect()

        # Memory leak due to cyclic dependency:
        # - The widget instance stores a `shared_ptr<Action>` in the Widget::action_ data member
        # - the action instance stores a PyObject that indirectly stores a `shared_ptr<Widget>`
        #
        self.assertEqual(widgetRefCounter.count, 1)  # pybind11 holder for `widget`
        self.assertEqual(actionRefCounter.count, 1)  # `widget.action_`

    def testFixMemoryLeak(self):
        widget = Widget()
        widget.name = "myWidget"
        widgetRefCounter = widget.refCounter()
        self.assertEqual(pyRefcount(widget), 1)
        self.assertEqual(widgetRefCounter.count, 1)

        action = Action()
        action.name = "myAction"
        action.setCallback(lambda x = widget.toWeak() : changeName(x))
        actionRefCounter = action.refCounter()
        self.assertEqual(pyRefcount(widget), 1)     # `widget` (the weak ptr in the lambda is a separate PyObject)
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(widgetRefCounter.count, 1) # pybind11 holder for `widget`
        self.assertEqual(actionRefCounter.count, 1) # pybind11 holder for `action`

        widget.action = action
        self.assertEqual(pyRefcount(widget), 1)     # `widget`
        self.assertEqual(pyRefcount(action), 1)     # `action`
        self.assertEqual(widgetRefCounter.count, 1) # pybind11 holder for `widget`
        self.assertEqual(actionRefCounter.count, 2) # pybind11 holder for `action` + `widget.action_`

        widget.triggerAction()
        self.assertEqual(widget.name, "newName")

        del action
        del widget
        gc.collect()

        # No memory leak!
        self.assertEqual(widgetRefCounter.count, 0)
        self.assertEqual(actionRefCounter.count, 0)


if __name__ == '__main__':
    unittest.main()
