#include <osgUtil/TangentSpaceGenerator>

#include <osg/Notify>

using namespace osgUtil;

TangentSpaceGenerator::TangentSpaceGenerator()
:	osg::Referenced(),
	T_(new osg::Vec4Array),
	B_(new osg::Vec4Array),
	N_(new osg::Vec4Array)
{
}

TangentSpaceGenerator::TangentSpaceGenerator(const TangentSpaceGenerator &copy, const osg::CopyOp &copyop)
:	osg::Referenced(copy),
	T_(static_cast<osg::Vec4Array *>(copyop(copy.T_.get()))),
	B_(static_cast<osg::Vec4Array *>(copyop(copy.B_.get()))),
	N_(static_cast<osg::Vec4Array *>(copyop(copy.N_.get())))
{
}

void TangentSpaceGenerator::generate(osg::Geometry *geo, int normal_map_tex_unit)
{
	const osg::Array *vx = geo->getVertexArray();
	const osg::Array *tx = geo->getTexCoordArray(normal_map_tex_unit);

	if (!vx || !tx) return;

	unsigned int vertex_count = vx->getNumElements();

	T_->assign(vertex_count, osg::Vec4());
	B_->assign(vertex_count, osg::Vec4());
	N_->assign(vertex_count, osg::Vec4());

	unsigned i; // VC6 doesn't like for-scoped variables

	for (i=0; i<geo->getNumPrimitiveSets(); ++i) {
		osg::PrimitiveSet *pset = geo->getPrimitiveSet(i);

		switch (pset->getMode()) {
			case osg::PrimitiveSet::TRIANGLES:
				for (i=0; i<pset->getNumIndices(); i+=3) {
					compute_basis_vectors(pset, vx, tx, i, i+1, i+2);
				}
				break;

			case osg::PrimitiveSet::TRIANGLE_STRIP:
				for (i=0; i<pset->getNumIndices()-2; ++i) {
					if ((i%2) == 0) {
						compute_basis_vectors(pset, vx, tx, i, i+1, i+2);
					} else {
						compute_basis_vectors(pset, vx, tx, i, i+2, i+1);
					}
				}
				break;

			case osg::PrimitiveSet::TRIANGLE_FAN:
				for (i=2; i<pset->getNumIndices(); ++i) {
					compute_basis_vectors(pset, vx, tx, 0, i-1, i);
				}
				break;

			case osg::PrimitiveSet::QUADS:
				for (i=0; i<pset->getNumIndices(); i+=4) {
					compute_basis_vectors(pset, vx, tx, i, i+1, i+2);
					compute_basis_vectors(pset, vx, tx, i+2, i+3, i);
				}
				break;

			case osg::PrimitiveSet::POINTS:
			case osg::PrimitiveSet::LINES:
			case osg::PrimitiveSet::LINE_STRIP:
			case osg::PrimitiveSet::LINE_LOOP:
				break;

			default: osg::notify(osg::WARN) << "Warning: TangentSpaceGenerator: unknown primitive mode " << pset->getMode() << "\n";
		}
	}

	// normalize basis vectors and force the normal vector to match
	// the triangle normal's direction

	for (i=0; i<vertex_count; ++i) {
		osg::Vec4 &vT = T_->at(i);
		osg::Vec4 &vB = B_->at(i);
		osg::Vec4 &vN = N_->at(i);
	
		osg::Vec3 txN = osg::Vec3(vT.x(), vT.y(), vT.z()) ^ osg::Vec3(vB.x(), vB.y(), vB.z());
		
		if (txN * osg::Vec3(vN.x(), vN.y(), vN.z()) >= 0) {
			vN = osg::Vec4(txN, 0);
		} else {
			vN = osg::Vec4(-txN, 0);
		}

		vT.normalize();
		vB.normalize();
		vN.normalize();
	}
}

void TangentSpaceGenerator::compute_basis_vectors(osg::PrimitiveSet *pset, const osg::Array *vx, const osg::Array *tx, int iA, int iB, int iC)
{
	iA = pset->index(iA);
	iB = pset->index(iB);
	iC = pset->index(iC);

	osg::Vec3 P1;
	osg::Vec3 P2;
	osg::Vec3 P3;

	int i; // VC6 doesn't like for-scoped variables

	switch (vx->getType())
	{
		case osg::Array::Vec2ArrayType:
			for (i=0; i<2; ++i) {
				P1.ptr()[i] = static_cast<const osg::Vec2Array *>(vx)->at(iA).ptr()[i];
				P2.ptr()[i] = static_cast<const osg::Vec2Array *>(vx)->at(iB).ptr()[i];
				P3.ptr()[i] = static_cast<const osg::Vec2Array *>(vx)->at(iC).ptr()[i];
			}
			break;

		case osg::Array::Vec3ArrayType:
			P1 = static_cast<const osg::Vec3Array *>(vx)->at(iA);
			P2 = static_cast<const osg::Vec3Array *>(vx)->at(iB);
			P3 = static_cast<const osg::Vec3Array *>(vx)->at(iC);
			break;

		case osg::Array::Vec4ArrayType:
			for (i=0; i<3; ++i) {
				P1.ptr()[i] = static_cast<const osg::Vec4Array *>(vx)->at(iA).ptr()[i];
				P2.ptr()[i] = static_cast<const osg::Vec4Array *>(vx)->at(iB).ptr()[i];
				P3.ptr()[i] = static_cast<const osg::Vec4Array *>(vx)->at(iC).ptr()[i];
			}
			break;

		default:
			osg::notify(osg::WARN) << "Warning: TangentSpaceGenerator: vertex array must be Vec2Array, Vec3Array or Vec4Array" << std::endl;
	}

	osg::Vec2 uv1;
	osg::Vec2 uv2;
	osg::Vec2 uv3;

	switch (tx->getType())
	{
		case osg::Array::Vec2ArrayType:
			uv1 = static_cast<const osg::Vec2Array *>(tx)->at(iA);
			uv2 = static_cast<const osg::Vec2Array *>(tx)->at(iB);
			uv3 = static_cast<const osg::Vec2Array *>(tx)->at(iC);
			break;

		case osg::Array::Vec3ArrayType:
			for (i=0; i<2; ++i) {
				uv1.ptr()[i] = static_cast<const osg::Vec3Array *>(tx)->at(iA).ptr()[i];
				uv2.ptr()[i] = static_cast<const osg::Vec3Array *>(tx)->at(iB).ptr()[i];
				uv3.ptr()[i] = static_cast<const osg::Vec3Array *>(tx)->at(iC).ptr()[i];
			}
			break;

		case osg::Array::Vec4ArrayType:
			for (i=0; i<2; ++i) {
				uv1.ptr()[i] = static_cast<const osg::Vec4Array *>(tx)->at(iA).ptr()[i];
				uv2.ptr()[i] = static_cast<const osg::Vec4Array *>(tx)->at(iB).ptr()[i];
				uv3.ptr()[i] = static_cast<const osg::Vec4Array *>(tx)->at(iC).ptr()[i];
			}
			break;

		default:
			osg::notify(osg::WARN) << "Warning: TangentSpaceGenerator: texture coord array must be Vec2Array, Vec3Array or Vec4Array" << std::endl;
	}

	osg::Vec3 face_normal = (P2 - P1) ^ (P3 - P1);

	osg::Vec3 V;

	V = osg::Vec3(P2.x() - P1.x(), uv2.x() - uv1.x(), uv2.y() - uv1.y()) ^ 
		osg::Vec3(P3.x() - P1.x(), uv3.x() - uv1.x(), uv3.y() - uv1.y());
	if (V.x() != 0) {
		V.normalize();
		T_->at(iA).x() += -V.y() / V.x();
		B_->at(iA).x() += -V.z() / V.x();
		T_->at(iB).x() += -V.y() / V.x();
		B_->at(iB).x() += -V.z() / V.x();
		T_->at(iC).x() += -V.y() / V.x();
		B_->at(iC).x() += -V.z() / V.x();			
	}

	V = osg::Vec3(P2.y() - P1.y(), uv2.x() - uv1.x(), uv2.y() - uv1.y()) ^ 
		osg::Vec3(P3.y() - P1.y(), uv3.x() - uv1.x(), uv3.y() - uv1.y());
	if (V.x() != 0) {
		V.normalize();
		T_->at(iA).y() += -V.y() / V.x();
		B_->at(iA).y() += -V.z() / V.x();
		T_->at(iB).y() += -V.y() / V.x();
		B_->at(iB).y() += -V.z() / V.x();
		T_->at(iC).y() += -V.y() / V.x();
		B_->at(iC).y() += -V.z() / V.x();			
	}

	V = osg::Vec3(P2.z() - P1.z(), uv2.x() - uv1.x(), uv2.y() - uv1.y()) ^ 
		osg::Vec3(P3.z() - P1.z(), uv3.x() - uv1.x(), uv3.y() - uv1.y());
	if (V.x() != 0) {
		V.normalize();
		T_->at(iA).z() += -V.y() / V.x();
		B_->at(iA).z() += -V.z() / V.x();
		T_->at(iB).z() += -V.y() / V.x();
		B_->at(iB).z() += -V.z() / V.x();
		T_->at(iC).z() += -V.y() / V.x();
		B_->at(iC).z() += -V.z() / V.x();			
	}

	N_->at(iA) += osg::Vec4(face_normal, 0);
	N_->at(iB) += osg::Vec4(face_normal, 0);
	N_->at(iC) += osg::Vec4(face_normal, 0);
}
