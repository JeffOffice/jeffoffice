# Verificar a ortografia
<!-- keywords: ortografia, verificar ortografia, verificador ortográfico, palavra mal escrita, erro ortográfico, linha vermelha, linha ondulada, sublinhado ondulado, sugestões, ignorar, ignorar tudo, adicionar ao dicionário, dicionário personalizado, dicionário, idioma, verificação, f7, rever, verificar ortografia ao escrever, maiúsculas, números, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática ao escrever -->

O JeffWrite verifica a ortografia à medida que escreve. Uma palavra que não esteja no dicionário fica com uma linha vermelha ondulada por baixo. A verificação é feita no seu próprio computador, sem ligação à Internet, e conhece 31 idiomas e variantes regionais.

## Enquanto escreve

- Uma linha vermelha ondulada indica que o dicionário não tem a palavra. A palavra pode, ainda assim, estar correta. Nomes e palavras novas costumam não constar do dicionário. Os nomes das aplicações JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC nunca são assinalados, em nenhum idioma, com ou sem "s".
- A palavra que está a escrever fica sem verificação até sair dela, para que uma palavra ainda incompleta não pisque.
- As linhas são apresentadas apenas no ecrã. Nunca são impressas nem incluídas num PDF.

Para corrigir uma palavra, clique com o botão direito do rato sobre ela. O menu mostra estas opções acima das habituais:

- Até seis **sugestões**, a melhor em primeiro lugar, em negrito. Clique numa para a colocar no lugar da palavra. Trata-se de um único passo, por isso **Ctrl+Z** a desfaz.
- **Ignorar Tudo** deixa essa palavra em paz, em todo o lado, até fechar o JeffWrite.
- **Adicionar ao Dicionário** ensina a palavra ao JeffWrite de forma permanente. A palavra é guardada num pequeno ficheiro na sua pasta pessoal do JeffOffice, e todas as aplicações JeffOffice a utilizam. Para ver ou alterar a sua lista de palavras, consulte As suas próprias palavras, mais abaixo.

Se o dicionário não tiver nenhuma sugestão, o menu indica-o.

## Percorrer os erros ortográficos

Prima **F7** ou clique em **Rever > Ortografia**. O painel Ortografia abre-se à direita. Mostra a primeira palavra mal escrita a seguir ao cursor, selecionada no texto, com a linha em que se encontra e uma lista de sugestões.

- **Alterar** coloca a sugestão escolhida no lugar da palavra. Faça duplo clique numa sugestão para fazer o mesmo.
- **Alterar Tudo** faz isso para todas as cópias da palavra nos locais por onde o painel passa, indicados abaixo. Trata-se de um único passo, por isso **Ctrl+Z** os desfaz todos, em qualquer local onde estivessem. Quando termina, o painel indica quantas palavras alterou e em quantos locais. O texto, cada caixa de texto, cada nota e cada cabeçalho ou rodapé contam como um local cada.
- **Ignorar** deixa passar esta palavra. A sua linha vermelha mantém-se.
- **Ignorar Tudo** e **Adicionar ao Dicionário** funcionam como no menu do botão direito.

O painel começa onde está o cursor: no texto, ou na caixa de texto, nota de rodapé, cabeçalho ou rodapé que estiver a editar. Percorre primeiro o texto do documento, incluindo as tabelas, depois as caixas de texto, depois as notas de rodapé e por fim os cabeçalhos e rodapés. Quando uma palavra está numa caixa de texto, nota de rodapé, cabeçalho ou rodapé, o JeffWrite abre-o para edição, como acontece na Procura, para que veja a palavra no sítio onde se encontra. Um cabeçalho ou rodapé que nenhuma página apresente é ignorado, e Alterar Tudo também o deixa como está. A partir do cursor, o painel vai até ao fim desse local, depois continua pelos restantes pela ordem indicada, e volta ao início até ao local onde começou. **Alterar** é um único passo que pode ser anulado, e **Alterar Tudo** procura em todos os locais por onde o painel passa, também como um único passo. Uma palavra numa nota de fim pode ser ignorada ou adicionada, mas não alterada. Quando não restarem mais palavras, o painel indica que a verificação ortográfica está concluída. Quando a ortografia estiver concluída, passa aos problemas de gramática e estilo (consulte Verificar gramática, mais abaixo).

## As suas próprias palavras

Clique em **Ficheiro > Opções > Verificação > Dicionário Personalizado** para ver e alterar as suas palavras. A caixa de diálogo tem duas listas, cada uma com uma caixa **Localizar** para as filtrar:

- **Palavras Adicionadas** são palavras que ensinou ao JeffWrite. São aceites em todos os idiomas.
- **Palavras Excluídas** são palavras que pretende ver assinaladas como erradas, mesmo que um dicionário as aceite, como uma palavra que escreve sempre mal. São assinaladas em todos os locais onde aparecem, com qualquer uso de maiúsculas, e nunca são sugeridas. Uma palavra excluída é apenas aquela palavra exata, e não as suas outras formas: excluir "casa" não assinala "casas". Adicionar uma palavra ao dicionário, a partir do painel, do menu do botão direito ou da caixa de diálogo, retira-a da lista de excluídas. **Ignorar Tudo** continua a funcionar numa palavra excluída até fechar o programa.

Escreva uma palavra e clique em **Adicionar**, ou selecione palavras e clique em **Remover**. Uma palavra só pode estar numa lista de cada vez: adicioná-la a uma retira-a da outra. **Importar** lê as palavras de um ficheiro de texto simples, com uma palavra por linha, e **Exportar** grava a lista no mesmo tipo de ficheiro (UTF-8). Uma linha com um espaço ou um carácter de controlo, ou com mais de 100 letras, fica de fora, e a caixa de diálogo indica quantas foram deixadas de fora. Uma lista pode ter até 1 MB de texto. Nada muda até clicar em **OK**. As listas são suas, são as mesmas em todas as aplicações JeffOffice e nunca são guardadas num documento.

## Quando a Correção Automática altera uma palavra

Quando a Correção Automática altera uma palavra enquanto escreve, aparece um pequeno botão **Opções de Correção Automática** por baixo do cursor. Tem três opções. Um leitor de ecrã indica que existem opções quando o botão aparece, e a descrição do botão diz como as abrir. Também pode premir **Alt+Shift+F10** para as abrir. Trata-se do comando Menu Opções de Correção Automática, que está na página Teclado das opções, junto dos outros comandos, onde lhe pode atribuir outra tecla.

- **Anular Correção Automática** repõe a palavra tal como a escreveu. Trata-se de um único passo, por isso **Refazer** volta a aplicar a alteração.
- Uma opção que impede a alteração. Depende do que alterou a palavra: **Parar de Substituir** retira a entrada da sua lista de substituições, **Manter Tal Como Escrito** acrescenta uma palavra com duas maiúsculas ("THursday") às palavras que as mantêm, e **Parar de Pôr em Maiúscula** desativa a maiúscula para os nomes dos dias, a primeira letra das frases ou a primeira letra das células de tabelas em todo o lado, e não apenas nesta palavra, e indica-o (é o mesmo interruptor da caixa de diálogo Correção Automática). Também repõe a palavra tal como a escreveu.
- **Opções de Correção Automática...** abre a caixa de diálogo Correção Automática.

O botão desaparece quando escreve, edita ou move o cursor.

## Ativar ou desativar as linhas vermelhas

Clique em **Rever > Verificar Ortografia ao Escrever**. A mesma opção está em **Ficheiro > Opções > Verificação**, e é a única opção que todas as aplicações JeffOffice partilham. Aí pode também optar por ignorar palavras em MAIÚSCULAS e palavras com números, como 3.º. Ambas são ignoradas por predefinição.

## Verificar gramática ao escrever

O JeffWrite também procura erros de gramática e de estilo evidentes no texto. Um problema fica com uma linha azul pontilhada por baixo. A ortografia mantém a sua linha vermelha ondulada, e uma palavra mal escrita nunca é assinalada como problema de gramática. Estes são os erros que procura:

- uma palavra repetida ("the the")
- "a" e "an" antes do som errado ("an test", "a hour")
- uma frase que começa com letra minúscula depois de um ponto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando as palavras à volta deixam o erro claro
- "could of" em vez de "could have"
- dois espaços seguidos dentro de uma frase
- um espaço antes de uma vírgula, de um ponto ou de outro sinal que pertença à palavra anterior
- falta de espaço depois do fim de uma frase
- "alot", e "everyday" em vez de "every day" ("almost everyday")
- verbos que não concordam com o sujeito em casos evidentes ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- a forma errada de um verbo ("did went", "has went", "can't hardly", "don't have no")
- palavras confundidas ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" em vez de "whether", "chose" em vez de "choose", "advise" em vez de "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- uma vírgula depois do ano de uma data ("On June 5, 2020, we left"), uma vírgula depois de uma cidade e de um estado ("in Austin, Texas, the heat"), e um hífen numa composição antes de um substantivo ("a well-known author")
- um "i" minúsculo em vez de "I", e os dias e meses em letra minúscula

Prefere deixar passar um erro a assinalar um texto correto, por isso uma frase sem linha azul não garante que esteja correta.

Clique com o botão direito do rato no problema. O menu indica o que está errado e depois uma ou mais sugestões. Clique numa sugestão para a colocar no texto, e prima Ctrl+Z para a anular. **Ignorar Este Problema** remove a linha apenas deste local, até fechar o JeffWrite. As mesmas palavras noutro local mantêm a sua linha, e editar outra parte do parágrafo não a faz voltar. **Ignorar Tudo** remove a linha de todos os problemas com esta mensagem nestas palavras.

A verificação é feita no seu próprio computador, sem ligação à Internet, em segundo plano: a escrita nunca é interrompida, e um parágrafo é verificado um pouco depois de deixar de escrever nele. Um parágrafo é verificado até aos seus primeiros 100 000 caracteres. Um documento que exija mais verificação do que o limite deixa de ser verificado, e a barra de estado avisa uma vez; a ortografia continua a funcionar.

**Que idiomas são verificados.** Inglês, francês, alemão e espanhol, cada um com regras próprias. O francês verifica o espaço inseparável antes de ; : ! ? e dentro de « », o apóstrofo em palavras como "l'ami" e "qu'il", e confusões como "a" e "à". O alemão verifica "das" e "dass", "seid" e "seit", "wider" e "wieder", a vírgula antes de "dass" e "weil", e as maiúsculas em substantivos comuns. O espanhol procura um ¿ ou ¡ em falta, acentos em palavras interrogativas, "a ver" e "haber", e "del" e "al". Um parágrafo é verificado no idioma em que está marcado, e o texto sem idioma é verificado como inglês. Num parágrafo com mais de um idioma, usa-se o primeiro idioma verificado e o resto é ignorado. O texto que um ficheiro marca como "não verificar" também é ignorado. A gramática é verificada no texto do documento, ainda não nas tabelas, cabeçalhos, rodapés ou notas.

Para a desativar, clique em **Rever > Verificar Gramática ao Escrever**. A mesma opção está em **Ficheiro > Opções > Verificação**. É uma única opção para todas as aplicações JeffOffice, e é a mesma em todas as janelas. O painel Ortografia (F7) percorre os problemas de gramática depois da ortografia, quando esta opção estiver ativada.

**Escolher o que é verificado.** Clique em **Ficheiro > Opções > Verificação > Definições de gramática** para escolher os tipos de problemas: **Gramática** (verbos que não concordam, a forma errada de um verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífens em excesso ou em falta), **Maiúsculas e Minúsculas**, **Palavras confundidas** (palavras que soam de forma parecida), **Repetição** e **Estilo**. **Estilo** está desativado até o ativar. Sugere melhorias e não corrige erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam pela mesma palavra e expressões prolixas ("in order to", "due to the fact that"). A escolha mantém-se ao clicar em **OK** nas Opções. Aplica-se de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas desta aplicação, e às outras aplicações JeffOffice na próxima vez que forem iniciadas.

## Idiomas

O JeffWrite verifica cada parte do texto no seu próprio idioma. Um ficheiro pode indicar em que idioma está o texto, até uma única palavra, e o JeffWrite lê essa indicação. O texto que não indica nada é verificado no idioma do documento. O ficheiro indica esse idioma para o documento inteiro, e um documento novo usa inglês americano.

O texto num idioma para o qual o JeffWrite não tem dicionário não é verificado. Também não é verificado o texto que um ficheiro marca como "não verificar ortografia". Ainda não é possível alterar o idioma de um texto no JeffWrite, mas mantém-se o que o ficheiro indica.

Os 31 dicionários são: inglês (americano, britânico, canadiano e australiano), espanhol (21 regiões, incluindo México e Espanha), francês, alemão, italiano, neerlandês e português (Brasil e Portugal).

## O que ainda não é verificado

As linhas vermelhas aparecem no texto, nas células das tabelas e num cabeçalho, rodapé ou nota de rodapé enquanto o edita. Ainda não aparecem nas caixas de texto, mas o painel Ortografia percorre-as.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
