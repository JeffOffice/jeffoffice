# Verificar a ortografia

<!-- keywords: ortografia, verificação ortográfica, verificador ortográfico, palavra mal escrita, linha vermelha, linha ondulada, sublinhado ondulado, sugestões, ignorar, ignorar tudo, adicionar ao dicionário, dicionário personalizado, dicionário, idioma, verificação, f7, rever, verificar ortografia ao escrever, maiúsculas, números, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática ao escrever -->

O JeffWrite verifica a sua ortografia à medida que escreve. Uma palavra que não está no dicionário fica com uma linha vermelha ondulada por baixo. A verificação é feita no seu próprio computador, sem ligação à Internet, e conhece 31 idiomas e variantes regionais.

## Enquanto escreve

- Uma linha vermelha ondulada significa que o dicionário não tem a palavra. A palavra pode, ainda assim, estar correta. Nomes e palavras novas costumam estar em falta. Os nomes das aplicações JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC nunca são assinalados, em nenhum idioma, com ou sem "s" no fim.
- A palavra que está a escrever fica sem marcação até sair dela, por isso uma palavra a meio não pisca.
- As linhas só aparecem no ecrã. Nunca são impressas nem incluídas num PDF.

Para corrigir uma palavra, **clique com o botão direito** nela. O menu mostra estas opções acima das habituais:

- Até seis **sugestões**, a melhor primeiro, em negrito. Clique numa para a colocar no lugar da palavra. É um único passo, por isso **Ctrl+Z** a desfaz.
- **Ignorar Tudo** deixa essa palavra em paz, em todo o lado, até fechar o JeffWrite.
- **Adicionar ao Dicionário** ensina a palavra ao JeffWrite de forma permanente. A palavra fica num pequeno ficheiro na sua pasta pessoal do JeffOffice, e todas as aplicações JeffOffice a usam. Para ver ou alterar a sua lista de palavras, consulte As suas próprias palavras, mais abaixo.

Se o dicionário não tiver nenhuma sugestão, o menu indica-o.

## Percorrer os erros ortográficos

Prima **F7**, ou clique em **Rever > Ortografia**. O painel Ortografia abre-se à direita. Mostra a primeira palavra mal escrita depois do cursor, selecionada no texto, com a linha em que se encontra e uma lista de sugestões.

- **Alterar** coloca a sugestão escolhida no lugar da palavra. Faça duplo clique numa sugestão para fazer o mesmo.
- **Alterar Tudo** faz isso para todas as cópias da palavra nos locais por onde o painel passa, indicados abaixo. É um único passo, por isso **Ctrl+Z** desfaz todas, onde quer que estivessem. Quando termina, o painel indica quantas palavras alterou e em quantos locais. O texto, cada caixa de texto, cada nota e cada cabeçalho ou rodapé contam como um local cada.
- **Ignorar** deixa passar esta palavra. A sua linha vermelha mantém-se.
- **Ignorar Tudo** e **Adicionar ao Dicionário** funcionam como no menu do botão direito.

O painel começa onde está o cursor: no texto, ou na caixa de texto, nota de rodapé, cabeçalho ou rodapé que estiver a editar. Percorre primeiro o texto do documento, incluindo as tabelas, depois as caixas de texto, depois as notas de rodapé e por fim os cabeçalhos e rodapés. Quando uma palavra está numa caixa de texto, nota de rodapé, cabeçalho ou rodapé, o JeffWrite abre-o para edição, como acontece na Localização, para que veja a palavra no sítio onde está. Um cabeçalho ou rodapé que nenhuma página mostra é ignorado, e **Alterar Tudo** também o deixa como está. A partir do cursor, o painel vai até ao fim desse local e depois continua pelos outros, por esta ordem, e volta ao início até ao local onde começou. **Alterar** é um passo que pode ser desfeito, e **Alterar Tudo** procura em todos os locais por onde o painel passa e também é um único passo. Uma palavra numa nota de fim pode ser ignorada ou adicionada, mas não alterada. Quando já não restarem mais, o painel indica que a verificação ortográfica está concluída. Depois da ortografia, passa aos problemas de gramática e estilo (consulte Verificar a gramática, mais abaixo).

## Contar os erros na barra de estado

A barra de estado mostra quantos erros ortográficos e gramaticais tem o documento, por exemplo **3 erros de revisão**. Clique nela para selecionar o erro seguinte ao cursor. Ao chegar ao último, um clique volta ao primeiro.

O JeffWrite conta enquanto trabalha, aos poucos, e volta a verificar apenas os parágrafos que alterar. Num documento longo, a contagem mostra um sinal de mais, por exemplo **120+ erros de revisão**, até todos os parágrafos terem sido verificados. Uma palavra que tanto a verificação ortográfica como a gramatical assinalem é contada duas vezes. Os erros gramaticais são contados nos parágrafos que a verificação gramatical já analisou, por isso esse número pode aumentar à medida que desloca o documento. A contagem fica oculta quando a verificação ortográfica e a gramatical estão ambas desativadas.

## As suas próprias palavras

Clique em **Ficheiro > Opções > Revisão > Dicionário Personalizado** para ver e alterar as suas palavras. A caixa de diálogo tem duas listas, cada uma com uma caixa **Localizar** para a restringir:

- **Palavras Adicionadas** são palavras que ensinou ao JeffWrite. São aceites em todos os idiomas.
- **Palavras Excluídas** são palavras que quer ver assinaladas como erradas, mesmo que um dicionário as aceite, como uma palavra que escreve sempre mal. São assinaladas onde quer que apareçam, com qualquer uso de maiúsculas, e nunca são sugeridas. Uma palavra excluída é exatamente essa palavra, e não as suas outras formas: excluir "casa" não assinala "casas". Adicionar uma palavra ao seu dicionário, a partir do painel, do menu do botão direito ou da caixa de diálogo, retira-a da lista de excluídas. **Ignorar Tudo** continua a funcionar numa palavra excluída até fechar o programa.

Escreva uma palavra e clique em **Adicionar**, ou selecione palavras e clique em **Remover**. Uma palavra fica numa só lista de cada vez: adicioná-la a uma retira-a da outra. **Importar** lê as palavras de um ficheiro de texto simples com uma palavra por linha, e **Exportar** grava a lista no mesmo tipo de ficheiro (UTF-8). Uma linha com um espaço ou um carácter de controlo, ou com mais de 100 letras, fica de fora, e a caixa de diálogo indica quantas foram omitidas. Uma lista pode ter até 1 MB de texto. Nada muda até clicar em **OK**. As listas são suas, são iguais em todas as aplicações JeffOffice e nunca ficam guardadas num documento.

## Quando a Correção Automática altera uma palavra

Quando a Correção Automática altera uma palavra enquanto escreve, aparece um pequeno botão **Opções de Correção Automática** abaixo do cursor. Tem três opções. Um leitor de ecrã anuncia que existem opções quando o botão aparece, e a descrição do botão indica como as abrir. Também pode premir **Alt+Shift+F10** para as abrir. É o comando **Menu Opções de Correção Automática**, por isso está na página Teclado das opções, juntamente com os outros comandos, e pode atribuir-lhe outra tecla aí.

- **Anular Correção Automática** repõe a palavra como a escreveu. É um único passo, por isso **Refazer** volta a fazer a alteração.
- Uma opção que impede a alteração. Depende do que alterou a palavra: **Parar de Substituir** retira a entrada da sua lista de substituições, **Manter Como Escrito** acrescenta uma palavra com duas maiúsculas ("THursday") às palavras que as mantêm, e **Parar de Pôr em Maiúscula** desliga a maiúscula dos nomes dos dias, da primeira letra das frases ou da primeira letra das células de tabelas em todo o lado, e não apenas para esta palavra, e indica-o (é o mesmo interruptor da caixa de diálogo Correção Automática). Também repõe a palavra como a escreveu.
- **Opções de Correção Automática...** abre a caixa de diálogo Correção Automática.

O separador **Símbolos Matemáticos** da caixa de diálogo Correção Automática tem um interruptor, **Substituir os nomes de símbolos matemáticos enquanto escreve**, que fica desativado até o ativar. Com ele ativo, uma barra invertida seguida de um nome, como \alpha, transforma-se no símbolo (α) quando escreve um espaço ou um sinal a seguir. O nome tem de começar uma palavra, por isso um caminho de pasta como C:\temp\pi fica como o escreveu. O separador lista todos os nomes. Quando um nome foi substituído, a opção do botão é **Parar de Substituir Nomes Matemáticos**, que desliga o interruptor. O JeffDeck e o JeffPub também os alteram; o JeffCrunch não altera nada numa célula.

O botão desaparece quando escreve, edita ou desloca o cursor.

## Desligar ou ligar as linhas vermelhas

Clique em **Rever > Verificar Ortografia ao Escrever**. A mesma opção está em **Ficheiro > Opções > Revisão**, e é a única opção que todas as aplicações JeffOffice usam. Aí pode também escolher ignorar palavras em MAIÚSCULAS e palavras com números, como 3.º. Ambas são ignoradas por predefinição.

## Verificar a gramática à medida que escreve

O JeffWrite procura também erros gramaticais e de estilo claros no texto. Um problema fica com uma linha azul pontilhada por baixo. A ortografia mantém a sua linha vermelha ondulada, e uma palavra mal escrita nunca é assinalada como problema gramatical. Estes são os erros que procura:

- uma palavra repetida ("o o")
- "um" e "uma" antes do som errado ("um hora", "uma amigo")
- uma frase que começa com letra minúscula depois de um ponto
- "seu" e "se", "ser" e "sê", "há" e "a", "mas" e "mais", e "onde" e "aonde", quando as palavras à volta tornam o erro claro
- "poderia de" em vez de "poderia ter"
- dois espaços seguidos dentro de uma frase
- um espaço antes de uma vírgula, de um ponto ou de outro sinal que pertença à palavra anterior
- nenhum espaço depois do fim de uma frase
- "alot", escrito como uma só palavra, e "todo dia" em vez de "todos os dias"
- verbos que não concordam com o sujeito em casos claros
- a forma errada de um verbo
- palavras confundidas (quem é o carro, um afeto sobre, menos pessoas, irregardless, não consigo respirar, "weather" em vez de "whether", "chose" em vez de "choose", "advise" em vez de "advice")
- "entre você e eu" com pronome errado, "Eu e ele fomos", "mais melhor" e "estas coisa"
- uma vírgula depois do ano numa data ("Em 5 de junho de 2020, saímos"), uma vírgula depois de uma cidade e estado ("em Lisboa, Portugal, o calor"), e um hífen num composto antes de um nome ("um autor bem conhecido")
- um "i" minúsculo para a palavra "I", e os dias e meses em minúsculas

Prefere deixar passar um erro a assinalar um texto que está certo, por isso uma frase sem linha azul não garante que está correta.

Clique com o botão direito no problema. O menu mostra o que está errado e depois uma ou mais sugestões. Clique numa sugestão para a colocar no texto, e prima Ctrl+Z para a desfazer. **Ignorar Este Problema** retira a linha apenas deste local, até fechar o JeffWrite. As mesmas palavras noutro sítio mantêm a sua linha, e editar outra parte do parágrafo não a faz voltar. **Ignorar Tudo** retira a linha de todos os problemas com esta mensagem nestas palavras.

A verificação é feita no seu próprio computador, sem ligação à Internet, em segundo plano: a escrita nunca fica à espera, e um parágrafo é verificado pouco depois de deixar de escrever nele. Um parágrafo é verificado até aos seus primeiros 100 000 carateres. Um documento que exija mais verificação do que o limite deixa de ser verificado, e a barra de estado avisa uma vez; a verificação ortográfica continua a funcionar.

**Que idiomas são verificados.** A gramática só é verificada em inglês, em todas as variantes regionais (americano, britânico, canadiano, australiano e as restantes). O texto em qualquer outro idioma é verificado apenas quanto à ortografia e não recebe marcas gramaticais. Um parágrafo é verificado no idioma em que está marcado, e o texto sem idioma é verificado como inglês. Num parágrafo com mais de um idioma, só o inglês é verificado e o resto é ignorado. O texto que um ficheiro marca como "não verificar" também é ignorado, tal como as palavras que uma alteração controlada eliminou, o texto oculto e os resultados de campos (como os números de página). A gramática é verificada no texto do documento, ainda não nas tabelas, cabeçalhos, rodapés e notas.

Para a desativar, clique em **Rever > Verificar Gramática ao Escrever**. A mesma opção está em **Ficheiro > Opções > Revisão**. É uma única opção para todas as aplicações JeffOffice, e é igual em todas as janelas. O painel Ortografia (F7) percorre os problemas gramaticais depois da ortografia, quando esta opção está ativa.

**Escolher o que procura.** Clique em **Ficheiro > Opções > Revisão > Definições de gramática** para escolher os tipos de problemas: **Gramática** (verbos que não concordam, forma errada de um verbo, "um" e "uma"), **Pontuação** (espaços, vírgulas e hífenes a mais ou em falta), **Maiúsculas e minúsculas**, **Palavras confundidas** (palavras que soam de forma semelhante), **Repetição** e **Estilo**. **Estilo** fica desativado até o ativar. Sugere melhorias e não corrige erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam com a mesma palavra e expressões prolixas ("a fim de", "devido ao facto de"). A escolha mantém-se quando clica em **OK** nas Opções. Aplica-se de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas desta aplicação, e às outras aplicações JeffOffice na próxima vez que forem iniciadas.

## Idiomas

O JeffWrite verifica cada bloco de texto no seu próprio idioma. Um ficheiro pode indicar em que idioma está o texto, até uma só palavra, e o JeffWrite lê essa indicação. O texto que não indica nenhum idioma é verificado no idioma do próprio documento. O ficheiro indica esse idioma para o documento inteiro, e um documento novo usa inglês americano.

O texto num idioma para o qual o JeffWrite não tem dicionário não é verificado. Também não é verificado o texto que um ficheiro marca como "não verificar a ortografia". Ainda não é possível alterar o idioma do texto no JeffWrite, mas mantém o que o ficheiro indica.

Os 31 dicionários são: inglês (americano, britânico, canadiano e australiano), espanhol (21 regiões, incluindo México e Espanha), francês, alemão, italiano, neerlandês e português (Brasil e Portugal).

## O que ainda não é verificado

As linhas vermelhas aparecem no texto, nas células das tabelas e num cabeçalho, rodapé ou nota de rodapé enquanto o edita. Ainda não aparecem nas caixas de texto, mas o painel Ortografia percorre-as.

## Ver também

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
