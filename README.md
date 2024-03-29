# [개인] Ray Tracing으로 구/평면 렌더링

**진행 기간**: 2015. 04. ~ 2015. 06.

**사용 언어/기술**: C++, (라이브러리 x)



## 소개

**광원**, 색을 가지는 구 또는 폴리곤 기반 평면의 **물체**들과 **카메라**를 특정 좌표에 배치하고,

화면 (1280\*960)의 각 픽셀마다 빛을 쏘아서 물체에 부딪히면 얻게되는 색과 반사되어 얻게되는 색들을 조합해 렌더링 시킨다.

반사 최대 횟수는 4번 내외이며 또한, 불투명한 물체에 부딪히면 반사를 시키지 않는다. 물체에 가로막혀 반사되지 않으면 그림자가 생긴다.



## 스크린샷

왼쪽에 평면과 가운데에 반투명한 구 4개를 배치하고 반사와 그림자 생성 확인.

![example1](./docs/example1.png)

투명한 구를 하나씩 위에서 떨어뜨려 봄

![example2](./docs/example2.png)

투명한 구 안에 색이 있는 반투명한 구들을 충돌 시킴

![example3](./docs/example3.png)