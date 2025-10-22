#include "DataConfig/Extra/Types/DcExtraTestFixtures.h"

void FDcExtraCoreTypesStruct::MakeFixture()
{
	GuidField1 = FGuid(1,2,3,4);
	GuidField2 = FGuid(0,0,0,0);

	Vec2Field1 = {0, 1};
	Vec2Field2 = FVector2D::ZeroVector;

	VecField1 = FVector::UpVector;
	VecField2 = FVector::ZeroVector;

	PlaneField1 = {1, 2, 3, 4};
	PlaneField2 = {0, 0, 0, 0};

	MatrixField1 = FMatrix::Identity;
	MatrixField2 = FMatrix{ForceInitToZero};

	RotatorField1 = FRotator(1, 2, 3);
	RotatorField2 = FRotator::ZeroRotator;

	QuatField1 = FQuat(1, 2, 3, 4);
	QuatField2 = FQuat(0, 0, 0, 0);

	TransformField1 = FTransform::Identity;
	TransformField2 = FTransform(FQuat::Identity, FVector(1,2,3), FVector::OneVector);

	ColorField1 = FColor::Black;
	ColorField2 = FColor::Blue;

	LinearColorField1 = FLinearColor::Black;
	LinearColorField2 = FLinearColor::White;

	IntPointField1 = FIntPoint(1, 2);
	IntPointField2 = FIntPoint(0, 0);

	IntVectorField1 = FIntVector(1, 2, 3);
	IntVectorField2 = FIntVector::ZeroValue;

	DateTimeField1 = FDateTime(0);
	DateTimeField2 = FDateTime(1988, 7, 23);

	TimespanField1 = FTimespan(0);
	TimespanField2 = FTimespan(5, 6, 7, 8);
}
