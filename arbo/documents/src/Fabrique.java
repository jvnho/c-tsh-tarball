import java.util.List;
import java.util.Collections;

import operator.*;

public class Fabrique{
    public static List<IUnaryOperator> listUnary(){
        return Collections.unmodifiableList(List.of(UnaryOperator.factorial(), UnaryOperator.sqrt(), UnaryOperator.not()));
    }
    public static List<IBinaryOperator> listBinary(){
        return Collections.unmodifiableList(List.of(BinaryOperator.addition(), BinaryOperator.and(), BinaryOperator.division(), 
        BinaryOperator.implication(), BinaryOperator.modulo(), BinaryOperator.multiplicaction(),
        BinaryOperator.nand(), BinaryOperator.nor(), BinaryOperator.or(), BinaryOperator.power(), BinaryOperator.substraction()));
    }
}