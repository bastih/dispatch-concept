class ABase : public Base {
  virtual void method() const {};
};

class BBase : public Base {};

class CBase : public Base {};

class A1Base final: public ABase {};
class A2Base final: public ABase {};
class A3Base final: public ABase {};
class A4Base final: public ABase {};
class A5Base final: public ABase {};
class A6Base final: public ABase {};
class A7Base final: public ABase {};
class A8Base final: public ABase {};

class B1Base final: public BBase {};
class B2Base final: public BBase {};
class B3Base final: public BBase {};
class B4Base final: public BBase {};
class B5Base final: public BBase {};
class B6Base final: public BBase {};
class B7Base final: public BBase {};
class B8Base final: public BBase {};

class C1Base final: public CBase {};
class C2Base final: public CBase {};
class C3Base final: public CBase {};
class C4Base final: public CBase {};
class C5Base final: public CBase {};
class C6Base final: public CBase {};
class C7Base final: public CBase {};
class C8Base final: public CBase {};

