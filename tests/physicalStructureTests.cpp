#include "testsMain.h"

#include "compare.h"
#include "../physics/misc/toString.h"

#include "randomValues.h"
#include "estimateMotion.h"

#include "../physics/geometry/shape.h"
#include "../physics/geometry/shapeCreation.h"
#include "../physics/part.h"
#include "../physics/physical.h"
#include "../physics/hardconstraints/fixedConstraint.h"

namespace P3D {
#define ASSERT(x) ASSERT_STRICT(x)

static CFrame cf() {
	return createRandomCFrame();
}

static Part* createPart() {
	return new Part(boxShape(1.0, 1.0, 1.0), GlobalCFrame(), {1.0, 1.0, 1.0});
}

TEST_CASE(testBasicCreateDestroy) {
	Part* p = createPart();
	p->ensureHasParent();
	delete p;
}

TEST_CASE(testManyAttachBasic) {
	Part* a = createPart();
	Part* b = createPart();
	Part* c = createPart();
	Part* d = createPart();

	a->attach(b, cf());
	a->attach(c, cf());
	c->attach(d, cf());

	// a should be mainPart
	ASSERT_TRUE(a->isMainPart());
	ASSERT_FALSE(b->isMainPart());
	ASSERT_FALSE(c->isMainPart());
	ASSERT_FALSE(d->isMainPart());

	delete b;
	delete a;
	delete c;
	delete d;
}

TEST_CASE(testManualDetach) {
	Part* a = createPart();
	Part* b = createPart();
	Part* c = createPart();
	Part* d = createPart();

	a->attach(b, cf());
	a->attach(c, cf());
	c->attach(d, cf());

	// a should be mainPart
	ASSERT_TRUE(a->isMainPart());
	ASSERT_FALSE(b->isMainPart());
	ASSERT_FALSE(c->isMainPart());
	ASSERT_FALSE(d->isMainPart());

	b->detach();
	ASSERT_TRUE(b->isMainPart());

	a->detach();
	ASSERT(c->parent->rigidBody.mainPart != a);

	delete b;
	delete a;
	delete c;
	delete d;
}

TEST_CASE(testManyAttachComplex) {
	Part* a = createPart();
	Part* b = createPart();
	Part* c = createPart();
	Part* d = createPart();

	Part* e1 = createPart();
	Part* e2 = createPart();
	e1->attach(e2, cf());

	Part* f1 = createPart();
	Part* f2 = createPart();
	f1->attach(f2, cf());

	Part* g1 = createPart();
	Part* g2 = createPart();
	g1->attach(g2, cf());

	// single new part attachments
	a->attach(b, cf());
	b->attach(c, cf());
	d->attach(b, cf());

	a->attach(e1, cf());
	a->attach(f2, cf());
	g1->attach(a, cf());

	ASSERT_TRUE(a->parent->rigidBody.getPartCount() == 10);

	Part* parts[]{a,b,c,d,e1,e2,f1,f2,g1,g2};
	Physical* parent = a->parent;
	for(Part* p : parts) {
		ASSERT_TRUE(p->parent == parent);
		delete p;
	}
}

TEST_CASE(testBasicMakeMainPhysical) {
	Part* a = createPart();
	Part* b = createPart();

	a->attach(b, new FixedConstraint(), cf(), cf());

	ASSERT_TRUE(a->parent != nullptr);
	ASSERT_TRUE(b->parent != nullptr);

	ASSERT_TRUE(a->parent->isMainPhysical());
	ASSERT_FALSE(b->parent->isMainPhysical());

	MotorizedPhysical* m = b->parent->mainPhysical;
	MotorizedPhysical* resultingMain = b->parent->makeMainPhysical();
	ASSERT_TRUE(resultingMain == m);

	ASSERT_TRUE(b->parent->isMainPhysical());
	ASSERT_FALSE(a->parent->isMainPhysical());

	ASSERT_TRUE(a->parent->childPhysicals.size() == 0);
	ASSERT_TRUE(b->parent->childPhysicals.size() == 1);
	ASSERT_TRUE(&b->parent->childPhysicals[0] == a->parent);
	ASSERT_TRUE(a->isValid());
	ASSERT_TRUE(b->isValid());
}

TEST_CASE(testAdvancedMakeMainPhysical) {
	Part* a = createPart();
	Part* b = createPart();
	Part* c = createPart();
	Part* d = createPart();
	Part* e = createPart();
	Part* f = createPart();
	Part* g = createPart();

	a->attach(b, new FixedConstraint(), cf(), cf());
	a->attach(e, new FixedConstraint(), cf(), cf());
	b->attach(c, new FixedConstraint(), cf(), cf());
	b->attach(d, new FixedConstraint(), cf(), cf());
	e->attach(f, new FixedConstraint(), cf(), cf());
	f->attach(g, new FixedConstraint(), cf(), cf());

	const int partCount = 7;
	Part* parts[partCount]{a,b,c,d,e,f,g};

	MotorizedPhysical* m = a->parent->mainPhysical;

	GlobalCFrame cframesBefore[partCount];
	for(int i = 0; i < partCount; i++) {
		cframesBefore[i] = parts[i]->getCFrame();
	}

	m->fullRefreshOfConnectedPhysicals();

	for(int i = 0; i < partCount; i++) {
		GlobalCFrame cfr = parts[i]->getCFrame();
		ASSERT_TOLERANT(cframesBefore[i] == cfr, 0.0005);
		cframesBefore[i] = cfr;
	}

	MotorizedPhysical* resultingMain = f->parent->makeMainPhysical();
	ASSERT_TRUE(resultingMain == m);
	ASSERT_TRUE(m->isValid());

	for(int i = 0; i < partCount; i++) {
		GlobalCFrame cfr = parts[i]->getCFrame();
		ASSERT_TOLERANT(cframesBefore[i] == cfr, 0.0005);
		cframesBefore[i] = cfr;
	}
	m->fullRefreshOfConnectedPhysicals();
	for(int i = 0; i < partCount; i++) {
		GlobalCFrame cfr = parts[i]->getCFrame();
		ASSERT_TOLERANT(cframesBefore[i] == cfr, 0.0005);
		cframesBefore[i] = cfr;
	}
}

TEST_CASE(testAttachConnectedPhysicalToConnectedPhysical) {
	Part* a = createPart();
	Part* b = createPart();
	Part* c = createPart();
	Part* d = createPart();
	Part* e = createPart();
	Part* f = createPart();
	Part* g = createPart();

	a->attach(b, new FixedConstraint(), cf(), cf());
	a->attach(e, new FixedConstraint(), cf(), cf());
	b->attach(c, new FixedConstraint(), cf(), cf());
	b->attach(d, new FixedConstraint(), cf(), cf());
	e->attach(f, new FixedConstraint(), cf(), cf());
	f->attach(g, new FixedConstraint(), cf(), cf());

	Part* a2 = createPart();
	Part* b2 = createPart();
	Part* c2 = createPart();
	Part* d2 = createPart();
	Part* e2 = createPart();
	Part* f2 = createPart();
	Part* g2 = createPart();

	a2->attach(b2, new FixedConstraint(), cf(), cf());
	a2->attach(e2, new FixedConstraint(), cf(), cf());
	b2->attach(c2, new FixedConstraint(), cf(), cf());
	b2->attach(d2, new FixedConstraint(), cf(), cf());
	e2->attach(f2, new FixedConstraint(), cf(), cf());
	f2->attach(g2, new FixedConstraint(), cf(), cf());

	e->attach(f2, new FixedConstraint(), cf(), cf());

	Part* parts[]{a,b,c,d,e,f,g,a2,b2,c2,d2,e2,f2,g2};

	MotorizedPhysical* mainPhys = a->parent->mainPhysical;

	for(Part* p : parts) {
		ASSERT_TRUE(p->parent->mainPhysical == mainPhys);
	}

	for(Part* p : parts) {
		delete p;
	}
}
};
