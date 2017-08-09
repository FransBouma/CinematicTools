#pragma once
#include "CameraHelpers.h"

class CameraManager
{
public:
  CameraManager();
  ~CameraManager();

  void Update(double dt);
  bool CameraHook(int pCamera);

  bool IsCameraEnabled() { return m_cameraEnabled; }

private:
  bool m_cameraEnabled;
  Camera m_camera;

  void UpdateCamera(double dt);
  void UpdateControls(double dt);

  ControlState m_controlState;
  bool m_keyboard;
  bool m_gamepad;

  RotationBuffer m_rotationHistory;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};