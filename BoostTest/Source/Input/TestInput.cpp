#include <boost/test/unit_test.hpp>
#include "../../../Client/Source/Input/Input.h"
#include "../../../Client/Source/ClientExceptions.h"

BOOST_AUTO_TEST_SUITE(InputTests)

class InputTranslatorStub : public InputTranslator
{
public:
	bool recordFunctionIsSet() { return m_RecordFunction; }
	void sendEvent(InputRecord& r) { m_RecordFunction(r); }
	bool handleFakeKeyboardInput(const RAWKEYBOARD& key)
		{ return handleKeyboardInput(key); }
	bool handleFakeMouseInput(const RAWMOUSE& mouse)
		{ return handleMouseInput(mouse); }
};

BOOST_AUTO_TEST_CASE(TestCreateInput)
{
	InputTranslatorStub* rawTranslator = new InputTranslatorStub;
	InputTranslatorStub::ptr fakeTranslator(rawTranslator);

	BOOST_CHECK(!rawTranslator->recordFunctionIsSet());

	Input input;
	input.init(std::move(fakeTranslator));

	BOOST_CHECK(rawTranslator->recordFunctionIsSet());

	input.destroy();
}

BOOST_AUTO_TEST_CASE(TestInputOnFrame)
{
	InputTranslatorStub* rawTranslator = new InputTranslatorStub;
	InputTranslatorStub::ptr fakeTranslator(rawTranslator);

	Input input;
	input.init(std::move(fakeTranslator));

	InputRecord rec;
	BOOST_REQUIRE(rawTranslator->recordFunctionIsSet());
	rawTranslator->sendEvent(rec);
	BOOST_CHECK_EQUAL(input.getFrameInputs().size(), 1);
	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 1);
	input.onFrame();
	BOOST_CHECK_EQUAL(input.getFrameInputs().size(), 0);
	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 1);

	input.destroy();
}

BOOST_AUTO_TEST_CASE(TestInputRegisteredKey)
{
	InputTranslatorStub* rawTranslator = new InputTranslatorStub;
	InputTranslatorStub::ptr fakeTranslator(rawTranslator);

	static const std::string spaceAction("testSpace");
	static const std::string pAction("testP");

	fakeTranslator->addKeyboardMapping(VK_SPACE, spaceAction);
	fakeTranslator->addKeyboardMapping('P', pAction);

	Input input;
	input.init(std::move(fakeTranslator));

	BOOST_REQUIRE(rawTranslator->recordFunctionIsSet());

	RAWKEYBOARD spacekey = {};
	spacekey.Flags = RI_KEY_MAKE;
	spacekey.VKey = VK_SPACE;

	RAWKEYBOARD pKey = {};
	pKey.Flags = RI_KEY_BREAK;
	pKey.VKey = 'P';

	BOOST_CHECK_EQUAL(rawTranslator->handleFakeKeyboardInput(spacekey), true);

	BOOST_REQUIRE_EQUAL(input.getFrameInputs().size(), 1);
	BOOST_CHECK_EQUAL(input.getFrameInputs()[0].m_Action, spaceAction);
	BOOST_CHECK_EQUAL(input.getFrameInputs()[0].m_Value, 1.f);

	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 1);
	BOOST_CHECK(input.getCurrentState().hasRecord(spaceAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(spaceAction), 1.f);
	
	BOOST_CHECK(!input.getCurrentState().hasRecord(pAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(pAction), 0.f);

	input.onFrame();

	BOOST_CHECK_EQUAL(input.getFrameInputs().size(), 0);

	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 1);
	BOOST_CHECK(input.getCurrentState().hasRecord(spaceAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(spaceAction), 1.f);
	
	BOOST_CHECK(!input.getCurrentState().hasRecord(pAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(pAction), 0.f);

	BOOST_CHECK_EQUAL(rawTranslator->handleFakeKeyboardInput(pKey), true);
	BOOST_CHECK_EQUAL(rawTranslator->handleFakeKeyboardInput(spacekey), true);

	BOOST_REQUIRE_EQUAL(input.getFrameInputs().size(), 2);
	BOOST_CHECK_EQUAL(input.getFrameInputs()[0].m_Action, pAction);
	BOOST_CHECK_EQUAL(input.getFrameInputs()[0].m_Value, 0.f);

	BOOST_CHECK_EQUAL(input.getFrameInputs()[1].m_Action, spaceAction);
	BOOST_CHECK_EQUAL(input.getFrameInputs()[1].m_Value, 1.f);

	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 2);
	BOOST_CHECK(input.getCurrentState().hasRecord(spaceAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(spaceAction), 1.f);
	
	BOOST_CHECK(input.getCurrentState().hasRecord(pAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(pAction), 0.f);

	input.onFrame();

	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 2);

	input.destroy();

	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 0);
}

BOOST_AUTO_TEST_CASE(TestInputTranslatorExc)
{
	InputTranslator inTrans;
	BOOST_CHECK_THROW(inTrans.init(nullptr), InvalidArgument);
}

BOOST_AUTO_TEST_CASE(TestInputRegisteredMouse)
{
	InputTranslatorStub* rawTranslator = new InputTranslatorStub;
	InputTranslatorStub::ptr fakeTranslator(rawTranslator);

	static const std::string moveUpAction("testMoveUp");
	static const std::string moveDownAction("testMoveDown");
	static const std::string moveLeftAction("testMoveLeft");
	static const std::string moveRightAction("testMoveRight");
	static const std::string leftButton("testLeftButton");
	static const std::string button5("testButton5");

	fakeTranslator->addMouseMapping(Axis::VERTICAL, true, moveUpAction);
	fakeTranslator->addMouseMapping(Axis::VERTICAL, false, moveDownAction);
	fakeTranslator->addMouseMapping(Axis::HORIZONTAL, true, moveRightAction);
	fakeTranslator->addMouseMapping(Axis::HORIZONTAL, false, moveLeftAction);
	fakeTranslator->addMouseButtonMapping(MouseButton::LEFT, leftButton);
	fakeTranslator->addMouseButtonMapping(MouseButton::EXTRA_2, button5);

	Input input;
	input.init(std::move(fakeTranslator));

	RAWMOUSE msg1 = {};
	msg1.lLastX = 0;
	msg1.lLastY = 0;
	msg1.usButtonFlags = RI_MOUSE_LEFT_BUTTON_DOWN;

	RAWMOUSE msg1_1 = {};
	msg1_1.lLastX = -10;
	msg1_1.lLastY = -5;

	RAWMOUSE msg2 = {};
	msg2.lLastX = 10;
	msg2.lLastY = 0;
	msg2.usButtonFlags = 0;

	RAWMOUSE msg3 = {};
	msg3.lLastX = 5;
	msg3.lLastY = 5;
	msg3.usButtonFlags = RI_MOUSE_BUTTON_5_UP;

	BOOST_CHECK_EQUAL(rawTranslator->handleFakeMouseInput(msg1), true);
	BOOST_CHECK_EQUAL(rawTranslator->handleFakeMouseInput(msg1_1), true);
	BOOST_CHECK_EQUAL(rawTranslator->handleFakeMouseInput(msg2), true);

	BOOST_CHECK_GE(input.getFrameInputs().size(), 4U);
	bool movedPosX = false;
	bool movedNegX = false;
	bool movedPosY = false;
	bool movedNegY = false;
	bool leftButtonPressed = false;
	bool button5Pressed = false;

	for (const InputRecord& rec : input.getFrameInputs())
	{
		if      (rec.m_Action == moveUpAction) movedNegY = true;
		else if (rec.m_Action == moveDownAction) movedPosY = true;
		else if (rec.m_Action == moveLeftAction) movedNegX = true;
		else if (rec.m_Action == moveRightAction) movedPosX = true;
		else if (rec.m_Action == leftButton) leftButtonPressed = true;
		else if (rec.m_Action == button5) button5Pressed = true;
		else
			BOOST_FAIL("Unknown action received: " + rec.m_Action);
	}

	BOOST_CHECK_EQUAL(movedNegX, true);
	BOOST_CHECK_EQUAL(movedPosX, true);
	BOOST_CHECK_EQUAL(movedNegY, true);
	BOOST_CHECK_EQUAL(movedPosY, false);
	BOOST_CHECK_EQUAL(leftButtonPressed, true);
	BOOST_CHECK_EQUAL(button5Pressed, false);

	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 4);
	BOOST_CHECK(input.getCurrentState().hasRecord(moveUpAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(moveUpAction), 5.f);
	BOOST_CHECK(input.getCurrentState().hasRecord(moveRightAction));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(moveRightAction), 10.f);
	
	BOOST_CHECK(!input.getCurrentState().hasRecord(button5));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(button5), 0.f);

	input.onFrame();

	BOOST_CHECK(rawTranslator->handleFakeMouseInput(msg3));

	BOOST_CHECK_EQUAL(input.getFrameInputs().size(), 3U);
	BOOST_CHECK_EQUAL(input.getCurrentState().size(), 6);
	BOOST_CHECK(input.getCurrentState().hasRecord(button5));
	BOOST_CHECK_EQUAL(input.getCurrentState().getValue(button5), 0.f);

	input.destroy();
}

BOOST_AUTO_TEST_SUITE_END()
