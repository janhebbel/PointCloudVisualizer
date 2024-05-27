#ifndef LINALG_H
#define LINALG_H

#include <stdint.h>
#include <math.h> // <- sqrtf

typedef struct
{
    uint32_t x;
    uint32_t y;
} v2u;

typedef struct
{
    float x;
    float y;
} v2f;

typedef struct
{
    float x;
    float y;
    float z;
} v3f;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} v4f;

typedef float m4x4[4][4];

typedef struct
{
    m4x4 p;
} mat4;

typedef struct
{
    float s;
    v3f   v;
} quat; // quaternion

#define PI 3.14159265359f
#define TABLE_SIZE 257
#define STEP_SIZE 0.25f * 2 * PI / (TABLE_SIZE - 1)

float table[TABLE_SIZE] = {
    0.0000000f, 0.0061359f, 0.0122715f, 0.0184067f, 0.0245412f, 0.0306748f, 
    0.0368072f, 0.0429383f, 0.0490677f, 0.0551952f, 0.0613207f, 0.0674439f, 
    0.0735646f, 0.0796824f, 0.0857973f, 0.0919090f, 0.0980171f, 0.1041216f, 
    0.1102222f, 0.1163186f, 0.1224107f, 0.1284981f, 0.1345807f, 0.1406582f, 
    0.1467305f, 0.1527972f, 0.1588582f, 0.1649131f, 0.1709619f, 0.1770042f, 
    0.1830399f, 0.1890687f, 0.1950903f, 0.2011046f, 0.2071114f, 0.2131103f, 
    0.2191012f, 0.2250839f, 0.2310581f, 0.2370236f, 0.2429802f, 0.2489276f, 
    0.2548656f, 0.2607941f, 0.2667128f, 0.2726214f, 0.2785197f, 0.2844076f, 
    0.2902847f, 0.2961509f, 0.3020059f, 0.3078496f, 0.3136818f, 0.3195020f, 
    0.3253103f, 0.3311063f, 0.3368898f, 0.3426607f, 0.3484187f, 0.3541635f, 
    0.3598951f, 0.3656130f, 0.3713172f, 0.3770074f, 0.3826835f, 0.3883450f, 
    0.3939920f, 0.3996242f, 0.4052413f, 0.4108432f, 0.4164295f, 0.4220003f, 
    0.4275551f, 0.4330938f, 0.4386162f, 0.4441221f, 0.4496113f, 0.4550836f, 
    0.4605387f, 0.4659765f, 0.4713967f, 0.4767992f, 0.4821838f, 0.4875502f, 
    0.4928982f, 0.4982277f, 0.5035384f, 0.5088302f, 0.5141027f, 0.5193560f, 
    0.5245897f, 0.5298036f, 0.5349976f, 0.5401715f, 0.5453250f, 0.5504580f, 
    0.5555702f, 0.5606616f, 0.5657318f, 0.5707808f, 0.5758082f, 0.5808140f, 
    0.5857978f, 0.5907597f, 0.5956993f, 0.6006165f, 0.6055110f, 0.6103828f, 
    0.6152316f, 0.6200572f, 0.6248595f, 0.6296383f, 0.6343933f, 0.6391245f, 
    0.6438316f, 0.6485144f, 0.6531729f, 0.6578067f, 0.6624158f, 0.6669999f, 
    0.6715590f, 0.6760927f, 0.6806010f, 0.6850837f, 0.6895406f, 0.6939715f, 
    0.6983762f, 0.7027547f, 0.7071068f, 0.7114322f, 0.7157308f, 0.7200025f, 
    0.7242470f, 0.7284644f, 0.7326543f, 0.7368166f, 0.7409511f, 0.7450578f, 
    0.7491364f, 0.7531868f, 0.7572088f, 0.7612024f, 0.7651673f, 0.7691033f, 
    0.7730104f, 0.7768885f, 0.7807372f, 0.7845566f, 0.7883464f, 0.7921066f, 
    0.7958369f, 0.7995373f, 0.8032075f, 0.8068475f, 0.8104572f, 0.8140363f, 
    0.8175848f, 0.8211025f, 0.8245893f, 0.8280451f, 0.8314696f, 0.8348629f, 
    0.8382247f, 0.8415549f, 0.8448536f, 0.8481203f, 0.8513552f, 0.8545580f, 
    0.8577286f, 0.8608669f, 0.8639728f, 0.8670462f, 0.8700870f, 0.8730950f, 
    0.8760701f, 0.8790122f, 0.8819212f, 0.8847971f, 0.8876396f, 0.8904487f, 
    0.8932243f, 0.8959663f, 0.8986744f, 0.9013488f, 0.9039893f, 0.9065957f, 
    0.9091680f, 0.9117060f, 0.9142098f, 0.9166790f, 0.9191138f, 0.9215140f, 
    0.9238795f, 0.9262102f, 0.9285061f, 0.9307670f, 0.9329928f, 0.9351835f, 
    0.9373390f, 0.9394592f, 0.9415441f, 0.9435934f, 0.9456074f, 0.9475856f, 
    0.9495282f, 0.9514350f, 0.9533060f, 0.9551412f, 0.9569404f, 0.9587035f, 
    0.9604305f, 0.9621214f, 0.9637761f, 0.9653944f, 0.9669765f, 0.9685221f, 
    0.9700313f, 0.9715039f, 0.9729400f, 0.9743394f, 0.9757021f, 0.9770281f, 
    0.9783174f, 0.9795697f, 0.9807853f, 0.9819639f, 0.9831055f, 0.9842101f, 
    0.9852777f, 0.9863081f, 0.9873014f, 0.9882576f, 0.9891765f, 0.9900582f, 
    0.9909027f, 0.9917098f, 0.9924796f, 0.9932119f, 0.9939070f, 0.9945646f, 
    0.9951847f, 0.9957674f, 0.9963126f, 0.9968203f, 0.9972904f, 0.9977230f, 
    0.9981181f, 0.9984756f, 0.9987954f, 0.9990777f, 0.9993224f, 0.9995294f, 
    0.9996988f, 0.9998306f, 0.9999247f, 0.9999812f, 1.0000000f
};

//
// trigonometric functions
float linalg_sin(float turn)
{
    float normalized_turn = turn - (int)turn;
    if (normalized_turn < 0.0f) normalized_turn = 1.0f + normalized_turn;
    
    int mirror = 0;
    int flip = 0;
    
    float index;
    if (normalized_turn >= 0.0f && normalized_turn < 0.25f)
    {
        index = normalized_turn * 4.0f * (TABLE_SIZE - 1);
    }
    else if (normalized_turn >= 0.25f && normalized_turn < 0.5f)
    {
        index = (normalized_turn - 0.25f) * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
    }
    else if (normalized_turn >= 0.5f && normalized_turn < 0.75f)
    {
        index = (normalized_turn - 0.5f) * 4.0f * (TABLE_SIZE - 1);
        flip = 1;
    }
    else
    {
        index = (normalized_turn - 0.75f) * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
        flip = 1;
    }
    if (mirror)
    {
        index = (TABLE_SIZE - 1) - index;
    }
    int index0 = (int)index;
    int index1 = index0 + 1;
    
    float lerp = table[index0] + (((table[index1] - table[index0]) / STEP_SIZE) *
                                  ((index - index0) * STEP_SIZE));
    
    if (flip)
    {
        return(-lerp);
    }
    else
    {
        return(lerp);
    }
}

float linalg_cos(float turn)
{
    float normalized_turn = turn - (int)turn;
    if (normalized_turn < 0.0f) normalized_turn = 1.0f + normalized_turn;
    
    int mirror = 0;
    int flip = 0;
    
    float index;
    if (normalized_turn >= 0.0f && normalized_turn < 0.25f)
    {
        index = normalized_turn * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
    }
    else if (normalized_turn >= 0.25f && normalized_turn < 0.5f)
    {
        index = (normalized_turn - 0.25f) * 4.0f * (TABLE_SIZE - 1);
        flip = 1;
    }
    else if (normalized_turn >= 0.5f && normalized_turn < 0.75f)
    {
        index = (normalized_turn - 0.5f) * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
        flip = 1;
    }
    else
    {
        index = (normalized_turn - 0.75f) * 4.0f * (TABLE_SIZE - 1);
    }
    if (mirror)
    {
        index = (TABLE_SIZE - 1) - index;
    }
    int index0 = (int)index;
    int index1 = index0 + 1;
    
    float lerp = table[index0] + (((table[index1] - table[index0]) / STEP_SIZE) *
                                  ((index - index0) * STEP_SIZE));
    
    if (flip)
    {
        return(-lerp);
    }
    else
    {
        return(lerp);
    }    
}

float linalg_tan(float turn)
{
    return(linalg_sin(turn) / linalg_cos(turn));
}

float linalg_cotan(float turn)
{
    return(linalg_cos(turn) / linalg_sin(turn));
}

//
// vector operations
v3f v3f_add(v3f v1, v3f v2)
{
    v3f result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return(result);
}

v3f v3f_sub(v3f v1, v3f v2)
{
	v3f result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return(result);
}

v3f v3f_scale(v3f v, float s)
{
    v3f result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    return(result);
}

float v3f_length(v3f v)
{
    return(sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
}

v3f v3f_normalize(v3f v)
{
	v3f result = {0};
    float length = v3f_length(v);
	if (length)
    {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}
	return(result);
}

v3f v3f_cross(v3f v1, v3f v2)
{
    v3f result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return(result);
}

float v3f_dot(v3f v1, v3f v2)
{
    return(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

v3f v3f_negate(v3f v)
{
    v3f result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return(result);
}

//
// matrix operations
mat4 mat4_identity()
{
    mat4 ident = {0};
    for(int i = 0; i < 4; ++i)
    {
        ident.p[i][i] = 1.0f;
    }
    return(ident);
}

mat4 mat4_mul(mat4 a, mat4 b)
{
	mat4 result;
	for (int i = 0; i < 4; ++i)
    {
		for (int j = 0; j < 4; ++j)
        {
            result.p[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k)
            {
				result.p[i][j] += a.p[i][k] * b.p[k][j];
			}
		}
	}
	return(result);
}

v4f mat4_mul_v4f(mat4 m, v4f v)
{
    float va[4] = {v.x, v.y, v.z, v.w};
    float ra[4] = {0.f};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			ra[i] += m.p[i][j] * va[j];
		}
	}
    v4f result = {ra[0], ra[1], ra[2], ra[3]};
	return(result);
}

mat4 mat4_transpose(mat4 m) {
    mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.p[i][j] = m.p[j][i];
        }
    }
    return(result);
}

//
// quaternion operations
// sources:
// https://www.haroldserrano.com/blog/developing-a-math-engine-in-c-implementing-quaternions
// https://github.com/haroldserrano/Revolver-4D-Math-Engine/blob/development/Revolver4DMathEngine/Revolver4DMathEngine/R4DQuaternion.mm
quat quat_add(quat a, quat b)
{
    quat result;
    result.s = a.s + b.s;
    result.v = v3f_add(a.v, b.v);
    return(result);
}

quat quat_sub(quat a, quat b)
{
    quat result;
    result.s = a.s - b.s;
    result.v = v3f_sub(a.v, b.v);
    return(result);
}

quat quat_scale(quat a, float s)
{
    quat result;
    result.s = a.s * s;
    result.v = v3f_scale(a.v, s);
    return(result);
}

quat quat_mul(quat a, quat b)
{
    // a * b = [a.s * b.s - a.v * b.v, s.a * b.v + s.b * a.v + a x b]
    quat result;
    result.s = (a.s * b.s) - v3f_dot(a.v, b.v);
    result.v = v3f_add(v3f_add(v3f_scale(b.v, a.s), v3f_scale(a.v, b.s)), v3f_cross(a.v, b.v));
    return(result);
}

float quat_norm(quat q)
{
    // ||q|| = sqrtf(q.s², s.v²);
    return(q.s * q.s + v3f_dot(q.v, q.v));
}

quat quat_normalize(quat q)
{
    // q / ||q||
    quat result = {0};
    float norm = quat_norm(q);
    if(norm)
    {
        result = quat_scale(q, 1 / norm);
    }
    return(result);
}

quat quat_conjugate(quat q)
{
    // q = [ q.s, -q.v ]
    quat result;
    result.s = q.s;
    result.v = v3f_scale(q.v, -1.0f);
    return(result);
}

quat quat_inverse(quat q)
{
    // q = conjugate(q) / norm(q)²
    float norm = quat_norm(q);
    float denominator = norm * norm;
    quat conjugate = quat_conjugate(q);
    return(quat_scale(conjugate, 1 / denominator));
}

//
// transformations
mat4 translate(v3f position)
{
    mat4 result = mat4_identity();
    result.p[0][3] = position.x;
    result.p[1][3] = position.y;
    result.p[2][3] = position.z;
    return(result);
}

// use this quaternion like this: p' = qpq*; q* = conjugate(q)
quat get_rotate_about_axis_quat(v3f axis, float turn)
{
    float half_turn = turn / 2.0f;
    
    quat result;
    result.s = linalg_cos(half_turn);
    result.v = v3f_scale(axis, linalg_sin(half_turn));
    return(result);
}

v3f v3f_rotate_about_axis_quat(v3f p, v3f axis, float turn)
{
    quat q = get_rotate_about_axis_quat(axis, turn);
    quat q_star = quat_conjugate(q);
    return(quat_mul(quat_mul(q, (quat){0.f, p}), q_star).v);
}

// source: https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix
mat4 mat4_rotate_quat(v3f axis, float turn)
{
    // NOTE: the axis vector has to be a unit vector
    quat q = get_rotate_about_axis_quat(axis, turn);
    
    mat4 m;
    m.p[0][0] = 1 - 2 * (q.v.y * q.v.y + q.v.z * q.v.z);
    m.p[0][1] = 2 * (q.v.x * q.v.y - q.v.z * q.s);
    m.p[0][2] = 2 * (q.v.x * q.v.z + q.v.y * q.s);
    m.p[0][3] = 0.0f;
    m.p[1][0] = 2 * (q.v.x * q.v.y + q.v.z * q.s);
    m.p[1][1] = 1 - 2 * (q.v.x * q.v.x + q.v.z * q.v.z);
    m.p[1][2] = 2 * (q.v.y * q.v.z - q.v.x * q.s);
    m.p[1][3] = 0.0f;
    m.p[2][0] = 2 * (q.v.x * q.v.z - q.v.y * q.s);
    m.p[2][1] = 2 * (q.v.y * q.v.z + q.v.x * q.s);
    m.p[2][2] = 1 - 2 * (q.v.x * q.v.x + q.v.y * q.v.y);
    m.p[3][0] = m.p[3][1] = m.p[3][2] = 0.0f;
    m.p[3][3] = 1.0f;
    return(m);
}

mat4 rotate_x_euler(float turn) {
    mat4 result = mat4_identity();
    result.p[1][1] =  linalg_cos(turn);
    result.p[1][2] = -linalg_sin(turn);
    result.p[2][1] =  linalg_sin(turn);
    result.p[2][2] =  linalg_cos(turn);
    return result;
}

mat4 rotate_y_euler(float turn) {
    mat4 result = mat4_identity();
    result.p[0][0] =  linalg_cos(turn);
    result.p[0][2] =  linalg_sin(turn);
    result.p[2][0] = -linalg_sin(turn);
    result.p[2][2] =  linalg_cos(turn);
    return result;
}

mat4 rotate_z_euler(float turn) {
    mat4 result = mat4_identity();
    result.p[0][0] =  linalg_cos(turn);
    result.p[0][1] = -linalg_sin(turn);
    result.p[1][0] =  linalg_sin(turn);
    result.p[1][1] =  linalg_cos(turn);
    return result;
}

mat4 scale(v3f s)
{
    mat4 result = {0};
    result.p[0][0] = s.x;
    result.p[1][1] = s.y;
    result.p[2][2] = s.z;
    result.p[3][3] = 1.0f;
    return(result);
}

//
// model, view, projection matrices
mat4 mat4_model(mat4 t, mat4 r, mat4 s)
{
    mat4 m = mat4_mul(mat4_mul(t, r), s);
    return(m);
}

mat4 look_at(v3f position, v3f target, v3f fake_up)
{
    // calculating forward direction of camera and negating it, so that the
    // camera points toward negative z
    v3f backward = v3f_normalize(v3f_sub(position, target));
    v3f right = v3f_normalize(v3f_cross(fake_up, backward));
    v3f up = v3f_cross(backward, right);
    
    mat4 result = {
        right.x,    right.y,    right.z,    -v3f_dot(right, position),
        up.x,       up.y,       up.z,       -v3f_dot(up, position),
        backward.x, backward.y, backward.z, -v3f_dot(backward, position),
        0.0f,       0.0f,       0.0f,       1.0f
    };
    return(result);
}

mat4 orthographic(float left, float right,
                  float bottom, float top,
                  float mnear, float mfar) {
    float width = right - left;
    float height = top - bottom;
    float depth = mfar - mnear;
    
    mat4 orth = {
        2.0f / width, 0.0f,          0.0f,          -(right + left) / width,
        0.0f,         2.0f / height, 0.0f,          -(top + bottom) / height,
        0.0f,         0.0f,          -2.0f / depth, -(mfar + mnear) / depth,
        0.0f,         0.0f,          0.0f,          1.0f
    };
	return orth;
}

mat4 perspective(float fov, float aspect, float mnear, float mfar)
{
    // fov in turns: 0 to 1 which is equivalent to 0 to 360 degrees
    float f = linalg_cotan(0.5f * fov);
    float inv_depth = 1.0f / (mnear - mfar);
    
    mat4 result = {
        f / aspect, 0.0f, 0.0f,                        0.0f,
        0.0f,       f,    0.0f,                        0.0f,
        0.0f,       0.0f, (mfar + mnear) * inv_depth,  (2.0f * mfar * mnear) * inv_depth,
        0.0f,       0.0f, -1.0f,                       0.0f
    };
    return(result);
}

#endif
