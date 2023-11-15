#pragma once
#include <iostream>
#include <assert.h>

#include <glm/matrix.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Quaternion
{
public:
	Quaternion() : i(1.0), j(0), k(0), w(0) {}
	Quaternion(float _i, float _j, float _k, float _w) : i(_i), j(_j), k(_k), w(_w) {}
	Quaternion& operator=(const Quaternion& other)
	{
		i = other.i;
		j = other.j;
		k = other.k;
		w = other.w;
		return *this;
	}
	~Quaternion() = default;

	void Print() const { std::cout << "(i,j,k,w) | (" << i << ", " << j << ", " << k << ", " << w << ")" << std::endl; }

	//Quaternion arithmetic
	Quaternion operator+(const Quaternion& rhs) const
	{
		return Quaternion(i + rhs.i, j + rhs.j, k + rhs.k, w + rhs.w);
	}

	void operator+=(const Quaternion& rhs)
	{
		i += rhs.i;
		j += rhs.j;
		k += rhs.k;
		w += rhs.w;
	}

	Quaternion operator-(const Quaternion& rhs) const
	{
		return Quaternion(i - rhs.i, j - rhs.j, k - rhs.k, w - rhs.w);
	}

	Quaternion operator-() const
	{
		return Quaternion(-i, -j, -k, -w);
	}

	//what does this do?
	Quaternion operator*(const Quaternion& rhs) const
	{
		return Quaternion(
			w*rhs.i + i * rhs.w + j * rhs.k - k * rhs.j,
			w*rhs.j + j * rhs.w - i * rhs.k + k * rhs.i,
			w*rhs.k + k * rhs.w + i * rhs.j - j * rhs.i,
			w*rhs.w - i * rhs.i - j * rhs.j - k * rhs.k);
	}

	Quaternion operator/(const Quaternion& rhs) const
	{
		return *this * rhs.inv();
	}

	float norm() const
	{
		return sqrt(i*i + j * j + k * k + w * w);
	}

	float normsquared() const
	{
		return (i*i + j * j + k * k + w * w);
	}

	Quaternion conj() const
	{
		return Quaternion(-i, -j, -k, w);
	}

	//if quaternion is normalized then inverse is just conjugate
	Quaternion inv() const
	{
		Quaternion conjugate = conj();
		float norm_2 = normsquared();

		return Quaternion(conjugate.i / norm_2, conjugate.j / norm_2,
			conjugate.k / norm_2, conjugate.w / norm_2);
	}

	void normalize()
	{
		float length = norm();
		i /= length;
		j /= length;
		k /= length;
		w /= length;
		assert(isnormalized());
	}

	bool isnormalized() const
	{
		if (std::abs(norm() - 1.0f) <= EPS)
			return true;
		else
			return false;
	}

	bool isPure() const
	{
		return w == 0;
	}

	bool isReal() const
	{
		return i == 0 && j == 0 && k == 0;
	}

	bool operator==(const Quaternion& rhs)
	{
		return (std::abs(i - rhs.i) <= EPS && std::abs(j - rhs.j) <= EPS && std::abs(k - rhs.k) <= EPS && std::abs(w - rhs.w) <= EPS)
			|| (std::abs(i + rhs.i) <= EPS && std::abs(j + rhs.j) <= EPS && std::abs(k + rhs.k) <= EPS && std::abs(w + rhs.w) <= EPS);
	}

	Quaternion Exp() const
	{
		// If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
		// exp(q) = e^w(cos(A)+sin(A)*(x*i+y*j+z*k)).  If sin(A) is near zero,
		// use exp(q) = e^w(cos(A)+(x*i+y*j+z*k)) since sin(A)/A has limit 1.

		float fAngle(std::sqrt(i*i + j * j + k * k));
		float fSin = std::sin(fAngle);
		float fExpW = std::exp(w);

		Quaternion kResult;
		kResult.w = fExpW * std::cos(fAngle);

		if (std::abs(fAngle) >= EPS)
		{
			float fCoeff = fExpW * (fSin / (fAngle));
			kResult.i = fCoeff * i;
			kResult.j = fCoeff * j;
			kResult.k = fCoeff * k;
		}
		else
		{
			kResult.i = fExpW * i;
			kResult.j = fExpW * j;
			kResult.k = fExpW * k;
		}

		return kResult;
	}

	Quaternion Log() const
	{
		//From Ogre OgreQuaternion.cpp
		// If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
		// log(q) = (A/sin(A))*(x*i+y*j+z*k).  If sin(A) is near zero, use
		// log(q) = (x*i+y*j+z*k) since A/sin(A) has limit 1.

		Quaternion kResult;
		kResult.w = 0.0;

		if (std::abs(w) < 1.0)
		{
			// According to Neil Dantam, atan2 has the best stability.
			// http://www.neil.dantam.name/note/dantam-quaternion.pdf
			float fNormV = std::sqrt(i*i + j * j + k * k);
			float fAngle(std::atan2(fNormV, w));

			float fSin = std::sin(fAngle);
			if (std::abs(fSin) >= EPS)
			{
				float fCoeff = fAngle / fSin;
				kResult.i = fCoeff * i;
				kResult.j = fCoeff * j;
				kResult.k = fCoeff * k;
				return kResult;
			}
		}

		kResult.i = i;
		kResult.j = j;
		kResult.k = k;

		return kResult;
	}

	//for quaternions is there any precisions issues to look for in operations/etc?
	void validateprecision() const
	{
		ContainsNaN();
	}

	bool ContainsNaN() const
	{
		if (isnan(i) || isnan(j) || isnan(k) || isnan(w)
			|| isinf(i) || isinf(j) || isinf(k) || isinf(w))
			return true;
		return false;
	}

	//shows how closely aligned two quaternions are. 1 means they are identical, -1 means they are in opposite directions
	float Dot(const Quaternion& rhs) const
	{
		return i * rhs.i + j * rhs.j + k * rhs.k + w * rhs.w;
	}

	//find angle between two quaternions
	float AngularDistance(const Quaternion& rhs) const
	{
		assert(isnormalized() && rhs.isnormalized());

		//From Unreal Quat.h
		float InnerProd = Dot(rhs);
		return std::acos((2 * InnerProd * InnerProd) - 1.0f);
	}

	//returns the angle of this quaternion
	float GetAngle() const
	{
		return 2.0f*std::acos(w);
	}

	//returns the rotation axis of this quaternion
	glm::vec3 GetRotationAxis() const
	{
		//equation from Ogre3D OgreQuaternion.cpp
		glm::vec3 rot;
		float SqrLength = normsquared();
		if (SqrLength > 0.0)
		{
			float invlength = 1.0 / norm();
			rot.x = i * invlength;
			rot.y = j * invlength;
			rot.z = k * invlength;
		}
		else
		{
			// angle is 0 (mod 2*pi), so any axis will do
			rot = glm::vec3(1.0, 0, 0);
		}
		return rot;
	}

	bool validateRotationMatrix(const glm::mat4& rot) const
	{
		//rotation matrix determinant should be 1
		assert(std::abs(glm::determinant(rot) - 1.0) <= EPS);

		//rotation matrix columns should be orthonormal basis (all unit vectors, and all orthogonal to eachother
		glm::vec3 e1(rot[0][0], rot[1][0], rot[2][0]);
		glm::vec3 e2(rot[0][1], rot[1][1], rot[2][1]);
		glm::vec3 e3(rot[0][2], rot[1][2], rot[2][2]);
		assert(std::abs(glm::length(e1) - 1.0) <= EPS && std::abs(glm::length(e2) - 1.0) <= EPS && std::abs(glm::length(e3) - 1.0) <= EPS);
		assert(std::abs(glm::dot(e1, e2)) <= EPS && std::abs(glm::dot(e1, e3)) <= EPS && std::abs(glm::dot(e2, e3)) <= EPS);

		return true;
	}

	//Quaternion combining is like matrices you just multiply and its right to left: second_Q * first_Q

   //Conversion helpers
	//quaternion has to be normalized
	glm::mat4 QtoMatrix() const
	{
		assert(isnormalized());
		glm::mat4 M;

		//From unreal engine QuatRotationTranslationMatrix.h
		const float x2 = i + i;  const float y2 = j + j;  const float z2 = k + k;
		const float xx = i * x2;   const float xy = i * y2;   const float xz = i * z2;
		const float yy = j * y2;   const float yz = j * z2;   const float zz = k * z2;
		const float wx = w * x2;   const float wy = w * y2;   const float wz = w * z2;

		/*//C-R
		M[0][0] = 1.0f - (yy + zz);	M[1][0] = xy - wz;				M[2][0] = xz + wy;			M[3][0] = 0;
		M[0][1] = xy + wz;			M[1][1] = 1.0f - (xx + zz);		M[2][1] = yz - wx;			M[3][1] = 0;
		M[0][2] = xz - wy;			M[1][2] = yz + wx;				M[2][2] = 1.0f - (xx + yy);	M[3][2] = 0;
		M[0][3] = 0.0f;				M[1][3] = 0.0f;					M[2][3] = 0.0f;				M[3][3] = 1.0f;
		*/
		//R-C
		M[0][0] = 1.0f - (yy + zz);	M[0][1] = xy - wz;				M[0][2] = xz + wy;			M[0][3] = 0;
		M[1][0] = xy + wz;			M[1][1] = 1.0f - (xx + zz);		M[1][2] = yz - wx;			M[1][3] = 0;
		M[2][0] = xz - wy;			M[2][1] = yz + wx;				M[2][2] = 1.0f - (xx + yy);	M[2][3] = 0;
		M[3][0] = 0.0f;				M[3][1] = 0.0f;					M[3][2] = 0.0f;				M[3][3] = 1.0f;

		validateRotationMatrix(M);

		return M;
	}

	void MatrixtoQ(glm::mat4 rot)
	{
		//first make sure its a valid rotation matrix
		validateRotationMatrix(rot);

		//from unreal engine Quat.h		
		float s;
		// Check diagonal (trace)
		const float tr = rot[0][0] + rot[1][1] + rot[2][2];

		if (tr > 0.0f)
		{
			float InvS = 1.0f / sqrt(tr + 1.0f);
			w = 0.5f * (1.0f / InvS);
			s = 0.5f * InvS;

			i = ((rot[2][1] - rot[1][2]) * s);
			j = ((rot[0][2] - rot[2][0]) * s);
			k = ((rot[1][0] - rot[0][1]) * s);
		}
		else
		{
			// diagonal is negative
			int32_t _i = 0;

			if (rot[1][1] > rot[0][0])
				_i = 1;

			if (rot[2][2] > rot[_i][_i])
				_i = 2;

			static constexpr int32_t nxt[3] = { 1, 2, 0 };
			const int32_t _j = nxt[_i];
			const int32_t _k = nxt[_j];

			s = rot[_i][_i] - rot[_j][_j] - rot[_k][_k] + 1.0f;

			float InvS = 1.0f / sqrt(s);

			float qt[4];
			qt[_i] = 0.5f * (1.f / InvS);

			s = 0.5f * InvS;

			qt[3] = (rot[_j][_k] - rot[_k][_j]) * s;
			qt[_j] = (rot[_i][_j] + rot[_j][_i]) * s;
			qt[_k] = (rot[_i][_k] + rot[_k][_i]) * s;

			i = qt[0];
			j = qt[1];
			k = qt[2];
			w = qt[3];
		}
	}

	//angle is in degrees
	//dividing angle by 2 because rotation rotates it by 2*angle
	void AAtoQ(glm::vec3 axis, float angle)
	{
		float real = cos(glm::radians(angle / 2.0f));
		glm::vec3 pure = axis * sin(glm::radians(angle / 2.0f));

		i = pure.x;
		j = pure.y;
		k = pure.z;
		w = real;
	}

	glm::vec4 QtoAA() const
	{
		glm::vec4 AA(GetRotationAxis(), GetAngle());
		return AA;
	}

	//for order x=pitch, y=head, roll=z. It should have values (1,2,3), (3,2,1), (1,3,2), etc
	void EulertoQ(float pitch /*x-matrix*/, float head /*y-matrix*/, float roll /*z-matrix*/, glm::ivec3 order)
	{
		Quaternion pitchQ(sin(glm::radians(pitch / 2)), 0, 0, cos(glm::radians(pitch / 2)));
		Quaternion headQ(0, sin(glm::radians(head / 2)), 0, cos(glm::radians(head / 2)));
		Quaternion rollQ(0, 0, sin(glm::radians(roll / 2)), cos(glm::radians(roll / 2)));

		Quaternion finalQ(0, 0, 0, 1);

		for (int j = 1; j <= 3; j++)
		{
			for (int i = 0; i < 3; i++)
			{
				if (order[i] == j)
				{
					if (i == 0)
						finalQ = pitchQ * finalQ;
					else if (i == 1)
						finalQ = headQ * finalQ;
					else if (i == 2)
						finalQ = rollQ * finalQ;
					break;
				}
			}
		}
		*this = finalQ;
		assert(isnormalized());
	}

	glm::vec3 QtoEuler() const
	{
	}

	//Useful functions
	 //creates quaternion such that: q*start = dest
	void QRotateAtoB(glm::vec3 start, glm::vec3 dest)
	{
		//math from Game Gems The Shortest Arc Quaternion
		start = glm::normalize(start);
		dest = glm::normalize(dest);

		//angle between them
		float cosTheta = glm::dot(start, dest);
		glm::vec3 rotationAxis;

		if (cosTheta < -1 + 0.001f) {
			// special case when vectors in opposite directions:
			// there is no "ideal" rotation axis
			// So guess one; any will do as long as it's perpendicular to start
			rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
			if (pow(glm::length(rotationAxis), 2) < 0.01) // bad luck, they were parallel, try again!
				rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

			rotationAxis = glm::normalize(rotationAxis);

			AAtoQ(rotationAxis, glm::radians(180.0f));
		}

		rotationAxis = glm::cross(start, dest);

		float s = sqrt((1.0 + cosTheta) * 2.0);
		float invs = 1.0 / s;

		i = rotationAxis.x*invs;
		j = rotationAxis.y*invs;
		k = rotationAxis.z*invs;
		w = s * .5f;

		normalize();
	}

	void LookAt(glm::vec3 forward, glm::vec3 up)
	{
		glm::vec3 z = glm::normalize(forward);
		glm::vec3 x = glm::cross(up, z);
		x = glm::normalize(x);
		glm::vec3 y = glm::cross(z, x);

		glm::mat3 m(x, y, z);
		glm::mat4 mm = m;

		MatrixtoQ(mm);
	}

	void RotateVector(glm::vec3& vec) const
	{
		assert(isnormalized());

		Quaternion pure_vec(vec.x, vec.y, vec.z, 0);

		Quaternion rotated_vec = Quaternion(i, j, k, w) * pure_vec * conj();

		assert(std::abs(rotated_vec.w) <= EPS);
		vec.x = rotated_vec.i;
		vec.y = rotated_vec.j;
		vec.z = rotated_vec.k;
	}

	//applies opposite angle rotation, which is the inverse of the quaternion (conjugate if its normalized)
	void UnRotateVector(glm::vec3& vec) const
	{
		assert(isnormalized());
		conj().RotateVector(vec);
	}

	//not normalized!
	static Quaternion Slerp(const Quaternion& Quat1, const Quaternion& Quat2, float alpha, bool shortestpath = false)
	{
		//From Unreal Engine UnrealMath.cpp
		// Get cosine of angle between quats.
		const float RawCosom =
			Quat1.i * Quat2.i +
			Quat1.j * Quat2.j +
			Quat1.k * Quat2.k +
			Quat1.w * Quat2.w;

		float Cosom = RawCosom;
		if (shortestpath)
			// Unaligned quats - compensate, results in taking shorter route.
			float Cosom = (RawCosom >= 0) ? RawCosom : -RawCosom;

		float Scale0, Scale1;

		if (Cosom < 0.9999f)
		{
			const float Omega = std::acos(Cosom);
			const float InvSin = 1.f / std::sin(Omega);
			Scale0 = std::sin((1.f - alpha) * Omega) * InvSin;
			Scale1 = std::sin(alpha * Omega) * InvSin;
		}
		else
		{
			// Use linear interpolation.
			Scale0 = 1.0f - alpha;
			Scale1 = alpha;
		}

		// In keeping with our flipped Cosom:
		if (shortestpath)
			Scale1 = (RawCosom >= 0) ? Scale1 : -Scale1;

		Quaternion Q;
		Q.i = Scale0 * Quat1.i + Scale1 * Quat2.i;
		Q.j = Scale0 * Quat1.j + Scale1 * Quat2.j;
		Q.k = Scale0 * Quat1.k + Scale1 * Quat2.k;
		Q.w = Scale0 * Quat1.w + Scale1 * Quat2.w;

		return Q;
	}

	//not normalized!
	static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float alpha)
	{
		// To ensure the 'shortest route', we make sure the dot product between the both rotations is positive.
		const float DotResult = q1.Dot(q2);
		const float Bias = (DotResult >= 0) ? 1.0f : -1.0f;
		return (q2 * Quaternion(0, 0, 0, alpha)) + (q1 * Quaternion(0, 0, 0, Bias * (1.f - alpha)));
	}

	static Quaternion Squad(const Quaternion& prevP, const Quaternion& P, const Quaternion& Q, const Quaternion& nextQ, float alpha)
	{
		//calculate tangents
		Quaternion tangent1;
		CalcTangents(prevP, P, Q, tangent1);
		Quaternion tangent2;
		CalcTangents(P, Q, nextQ, tangent2);

		//do Squad calculation
		const Quaternion Q1 = Slerp(P, Q, alpha);
		const Quaternion Q2 = Slerp(tangent1, tangent2, alpha);
		Quaternion Result = Slerp(Q1, Q2, 2.f * alpha * (1.f - alpha));
		Result.normalize();

		return Result;
	}

	static Quaternion CalcTangents(const Quaternion& PrevP, const Quaternion& P, const Quaternion& NextP, Quaternion& OutTan)
	{
		const Quaternion InvP = P.inv();
		const Quaternion Part1 = (InvP * PrevP).Log();
		const Quaternion Part2 = (InvP * NextP).Log();

		const Quaternion PreExp = (Part1 + Part2) * Quaternion(0, 0, 0, -0.5f);

		OutTan = P * PreExp.Exp();
	}

	//create test program and test quaternions: take two vectors and make one vector rotate to other. add two vectors. take two rotations, multiply and it should add the rotations.
	//make an object always rotate towards center. Make camera rotation work with roll/pitch/yaw. Do euler calculations with quaternion. 

	/*
		The point of Quaternions. Once you get the arithmetic and conversions, really the importance of Quaternions comes from the fact you can look at a unit quaternion as
		an axis and an angle. This represents the orientation of rotating around that axis at that angle. With the method you can now rotate a vector by that. Also combining
		quaternions simply combines the rotations. And quaternions are nice for linearly interpolating, they can represent any orientation robustly, require less space than matrix,
		and can be converted to matrices at the end of the day.
	*/

	const float EPS = 0.000001;
	static Quaternion Identity()
	{
		return Quaternion(0, 0, 0, 1);
	}
private:
	float i, j, k, w;
};