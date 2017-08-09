#include "CameraManager.h"
#include "../AlienIsolation.h"
#include "../Main.h"
#include "../Util/Log.h"

using namespace DirectX;

CameraManager::CameraManager()
{
  m_camera.pitch = m_camera.yaw = m_camera.roll = 0;
  XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(0, 0, 0);
  XMStoreFloat4(&m_camera.qRotation, rotation);
  m_camera.x = 2.06f;
  m_camera.y = 14.85f;
  m_camera.z = 37.f;
  m_camera.fov = 45.f;

  ZeroMemory(&m_controlState, sizeof(ControlState));
  ZeroMemory(&m_rotationHistory, sizeof(RotationBuffer));

  m_cameraEnabled = true;
  Log::Ok("Camera Manager initialized");
}

void CameraManager::Update(double dt)
{
  if (!m_cameraEnabled) return;

  UpdateControls(dt);

  // Average rotation from 100 updates to smooth
  // rotating movements
  double averagePitch = 0, averageYaw = 0, averageRoll = 0;
  for(int i = 0; i<100; ++i)
  {
    averagePitch += m_rotationHistory.pitch[i] / 100;
    averageYaw += m_rotationHistory.yaw[i] / 100;
    averageRoll += m_rotationHistory.roll[i] / 100;
  }

  m_camera.pitch += averagePitch;
  m_camera.yaw += averageYaw;
  m_camera.roll += averageRoll;

  UpdateCamera(dt);
}

void CameraManager::UpdateCamera(double dt)
{
  XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(m_camera.pitch, m_camera.yaw, m_camera.roll);
  XMVECTOR transform = XMVectorSet(m_camera.x, m_camera.y, m_camera.z, 0);

  XMVECTOR left = XMVectorSet(1, 0, 0, 0);
  XMVECTOR up = XMVectorSet(0, 1, 0, 0);
  XMVECTOR forward = XMVectorSet(0, 0, 1, 0);

  transform += XMVector3Rotate(left, rotation) * m_controlState.dX * m_camera.movementSpeed * dt;
  transform += XMVector3Rotate(up, rotation) * m_controlState.dY * m_camera.movementSpeed * dt;
  transform += XMVector3Rotate(forward, rotation) * m_controlState.dZ * m_camera.movementSpeed * dt;

  XMStoreFloat4(&m_camera.qRotation, rotation);
  m_camera.x = transform.m128_f32[0];
  m_camera.y = transform.m128_f32[1];
  m_camera.z = transform.m128_f32[2];

  m_controlState.dX = 0;
  m_controlState.dY = 0;
  m_controlState.dZ = 0;
  m_controlState.dPitch = 0;
  m_controlState.dYaw = 0;
  m_controlState.dRoll = 0;
}

void CameraManager::UpdateControls(double dt)
{
  for(int i = 99; i>0; --i)
  {
    m_rotationHistory.pitch[i] = m_rotationHistory.pitch[i - 1];
    m_rotationHistory.yaw[i] = m_rotationHistory.yaw[i - 1];
    m_rotationHistory.roll[i] = m_rotationHistory.roll[i - 1];
  }


  if (!AI::Rendering::HasFocus())
    return;
  
  GamepadState state;
  memset(&state, 0, sizeof(GamepadState));

  if (g_mainHandle->GetInputManager()->GetGamepadState(state))
  {
    m_controlState.dX += state.leftStick.x;
    m_controlState.dY += state.trigger;
    m_controlState.dZ -= state.leftStick.y;

    m_controlState.dPitch -= state.rightStick.y;
    m_controlState.dYaw -= state.rightStick.x;

    if (state.leftShoulder)
      m_controlState.dRoll += 1;
    if (state.rightShoulder)
      m_controlState.dRoll -= 1;

    if (state.rightThumbButton)
      m_camera.fov -= 5 * dt;
    if (state.leftThumbButton)
      m_camera.fov += 5 * dt;
  }

  if (GetAsyncKeyState('W') & 0x8000)
    m_controlState.dZ += 1;
  if (GetAsyncKeyState('S') & 0x8000)
    m_controlState.dZ -= 1;
  if (GetAsyncKeyState('A') & 0x8000)
    m_controlState.dX += 1;
  if (GetAsyncKeyState('D') & 0x8000)
    m_controlState.dX -= 1;
  if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    m_controlState.dY += 1;
  if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
    m_controlState.dY -= 1;

  m_rotationHistory.pitch[0] = m_controlState.dPitch * dt * m_camera.rotationSpeed;
  m_rotationHistory.yaw[0] = m_controlState.dYaw * dt * m_camera.rotationSpeed;
  m_rotationHistory.roll[0] = m_controlState.dRoll * dt * m_camera.rollSpeed;
}



bool CameraManager::CameraHook(int pCamera)
{
  if (!m_cameraEnabled) return false;

  XMFLOAT4* pRotation = (XMFLOAT4*)(pCamera);
  XMFLOAT3* pTransform = (XMFLOAT3*)(pCamera + 0x10);
  float* pFov = (float*)pCamera + 0x1C;

  *pRotation = m_camera.qRotation;
  pTransform->x = m_camera.x;
  pTransform->y = m_camera.y;
  pTransform->z = m_camera.z;
  *pFov = m_camera.fov;

  return true;
}

CameraManager::~CameraManager()
{
  
}