# Verificar sua ortografia
<!-- keywords: ortografia, verificar ortografia, corretor ortográfico, palavra incorreta, palavra errada, linha vermelha, linha ondulada, sublinhado ondulado, sugestões, ignorar, ignorar tudo, adicionar ao dicionário, dicionário personalizado, dicionário, idioma, revisão de texto, f7, revisão, verificar ortografia ao digitar, maiúsculas, números, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática ao digitar -->

O JeffWrite verifica sua ortografia enquanto você digita. Uma palavra que não está no dicionário recebe uma linha vermelha ondulada embaixo. A verificação é feita no seu próprio computador, sem conexão com a internet, e conhece 31 idiomas e variantes regionais.

## Enquanto você digita

- Uma linha vermelha ondulada significa que o dicionário não tem a palavra. A palavra pode estar certa. Nomes e palavras novas costumam não estar no dicionário. Os nomes dos aplicativos do JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e "LLC" nunca são marcados, em nenhum idioma, com ou sem "s" no final.
- A palavra que você está digitando é deixada em paz até você passar para a próxima, para que uma palavra pela metade não fique piscando.
- As linhas aparecem somente na tela. Elas nunca são impressas e nunca vão para um PDF.

Para corrigir uma palavra, **clique com o botão direito** nela. O menu mostra estas opções acima das habituais:

- Até seis **sugestões**, a melhor primeiro, em negrito. Clique em uma para colocá-la no lugar da palavra. É uma única etapa, então **Ctrl+Z** a desfaz.
- **Ignorar Tudo** deixa essa palavra em paz, onde quer que apareça, até você fechar o JeffWrite.
- **Adicionar ao Dicionário** ensina a palavra ao JeffWrite de forma definitiva. A palavra vai para um pequeno arquivo na sua própria pasta do JeffOffice, e todos os aplicativos do JeffOffice a usam. Para ver ou alterar sua lista de palavras, veja Suas próprias palavras, abaixo.

Se o dicionário não tiver nenhuma sugestão, o menu informa isso.

## Percorrer os erros de ortografia

Pressione **F7** ou clique em **Revisão > Ortografia**. O painel Ortografia abre à direita. Ele mostra a primeira palavra incorreta depois do cursor, selecionada no seu texto, com a linha em que está e uma lista de sugestões.

- **Alterar** coloca a sugestão escolhida no lugar da palavra. Clique duas vezes em uma sugestão para fazer o mesmo.
- **Alterar Tudo** faz isso para cada ocorrência da palavra nos locais que o painel percorre, listados abaixo. É uma única etapa, então **Ctrl+Z** desfaz todas elas, onde quer que estivessem. Ao terminar, o painel informa quantas palavras foram alteradas e em quantos locais. O texto, cada caixa de texto, cada nota e cada cabeçalho ou rodapé contam como um local cada.
- **Ignorar** deixa esta palavra passar. Sua linha vermelha permanece.
- **Ignorar Tudo** e **Adicionar ao Dicionário** funcionam como no menu do botão direito.

O painel começa onde está o cursor: no texto, ou na caixa de texto, nota de rodapé, cabeçalho ou rodapé que você está editando. Ele percorre primeiro o texto do documento, inclusive as tabelas, depois as caixas de texto, depois as notas de rodapé e por fim os cabeçalhos e rodapés. Quando a palavra está em uma caixa de texto, nota de rodapé, cabeçalho ou rodapé, o JeffWrite a abre para edição, como faz a Localizar, para que você veja a palavra no lugar em que ela está. Um cabeçalho ou rodapé que nenhuma página exibe é ignorado, e **Alterar Tudo** também o deixa como está. A partir do cursor, o painel vai até o fim desse local, depois segue pelos outros nesta ordem e volta ao início, até o ponto em que começou. **Alterar** é uma etapa que pode ser desfeita, e **Alterar Tudo** procura em todos os locais percorridos pelo painel e também é uma única etapa. Uma palavra em uma nota de fim pode ser ignorada ou adicionada, mas não alterada. Quando não restarem mais palavras, o painel informa que a verificação ortográfica foi concluída. Quando a ortografia termina, ele passa para os problemas de gramática e estilo (veja Verificar gramática, abaixo).

## Contar os erros na barra de status

A barra de status mostra quantos erros de ortografia e gramática o documento tem, como **3 erros de revisão**. Clique nela para selecionar o próximo erro depois do cursor. Ao chegar ao último, um clique volta ao primeiro.

O JeffWrite faz a contagem enquanto você trabalha, aos poucos, e verifica novamente somente os parágrafos que você altera. Em um documento longo, a contagem mostra um sinal de mais, como **120+ erros de revisão**, até que todos os parágrafos tenham sido verificados. Uma palavra que é marcada tanto pela verificação ortográfica quanto pela gramatical é contada duas vezes. Os erros de gramática são contados para os parágrafos que a verificação gramatical já analisou, então esse número pode aumentar conforme você rola o texto. A contagem fica oculta quando as verificações de ortografia e de gramática estão ambas desativadas.

## Suas próprias palavras

Clique em **Arquivo > Opções > Revisão de Texto > Dicionário Personalizado** para ver e alterar suas palavras. A caixa de diálogo tem duas listas, cada uma com uma caixa **Localizar** para filtrá-la:

- **Palavras Adicionadas** são palavras que você ensinou ao JeffWrite. Elas são aceitas em todos os idiomas.
- **Palavras Excluídas** são palavras que você quer marcadas como incorretas, mesmo que um dicionário as aceite, como uma palavra que você sempre digita errado. Elas são marcadas onde quer que apareçam, em qualquer uso de maiúsculas, e nunca são oferecidas como sugestão. Uma palavra excluída é exatamente aquela palavra, não suas outras formas: excluir "casa" não marca "casas". Adicionar uma palavra ao seu dicionário, pelo painel, pelo menu do botão direito ou pela caixa de diálogo, a retira da lista de excluídas. **Ignorar Tudo** ainda funciona para uma palavra excluída até você fechar o programa.

Digite uma palavra e clique em **Adicionar**, ou selecione palavras e clique em **Remover**. Uma palavra fica em apenas uma lista por vez: adicioná-la a uma lista a tira da outra. **Importar** lê as palavras de um arquivo de texto simples, com uma palavra por linha, e **Exportar** grava a lista como o mesmo tipo de arquivo (UTF-8). Uma linha que tenha espaço ou caractere de controle, ou que tenha mais de 100 letras, é deixada de fora, e a caixa de diálogo informa quantas foram. Uma lista pode ter até 1 MB de texto. Nada muda até você clicar em **OK**. As listas são suas, são as mesmas em todos os aplicativos do JeffOffice e nunca são salvas em um documento.

## Quando a Autocorreção altera uma palavra

Quando a Autocorreção altera uma palavra enquanto você digita, um pequeno botão **Opções de Autocorreção** aparece abaixo do cursor. Ele tem três opções. Um leitor de tela informa que há opções quando o botão aparece, e a descrição do botão diz como abri-las. Você também pode pressionar **Alt+Shift+F10** para abri-las. É o comando **Menu de Opções de Autocorreção**, que fica na página Teclado das opções, junto com os outros comandos, e você pode atribuir outra tecla a ele ali.

- **Desfazer Correção Automática** devolve a palavra como você a digitou. É uma única etapa, então **Refazer** faz a alteração novamente.
- Uma opção que interrompe a alteração. Depende do que alterou a palavra: **Parar de Substituir** retira a entrada da sua lista de substituições, **Manter como Digitado** adiciona uma palavra com duas letras maiúsculas ("THursday") às palavras que as mantêm, e **Parar de Usar Maiúscula** desativa a maiúscula para nomes de dias, a primeira letra de frases ou a primeira letra de células de tabela em todo o documento, e não só para esta palavra, e informa isso (é a mesma chave da caixa de diálogo Autocorreção). Ela também devolve a palavra como você a digitou.
- **Opções de Autocorreção...** abre a caixa de diálogo Autocorreção.

A guia **Símbolos Matemáticos** da caixa de diálogo Autocorreção tem uma chave, **Substituir os nomes de símbolos matemáticos ao digitar**, que fica desativada até você ligá-la. Com ela ligada, uma barra invertida seguida de um nome, como \alpha, vira o símbolo (α) quando você digita um espaço ou um sinal depois dele. O nome precisa começar uma palavra, então um caminho de pasta como C:\temp\pi permanece como você digitou. A guia lista todos os nomes. Quando um nome foi substituído, a opção do botão é **Parar de Substituir Nomes Matemáticos**, que desliga a chave. O JeffDeck e o JeffPub também fazem essas substituições; o JeffCrunch não altera nada em uma célula.

O botão desaparece quando você digita, edita ou move o cursor.

## Ligar ou desligar as linhas vermelhas

Clique em **Revisão > Verificar Ortografia ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**, e é a única opção que todos os aplicativos do JeffOffice usam. Ali você também pode escolher ignorar palavras em MAIÚSCULAS e palavras com números, como 3º. Ambas são ignoradas no início.

## Verificar gramática ao digitar

O JeffWrite também procura erros claros de gramática e de estilo no texto. Um problema recebe uma linha azul pontilhada embaixo. A ortografia mantém sua linha vermelha ondulada, e uma palavra incorreta nunca é marcada como problema de gramática. São estes os erros que ele procura:

- uma palavra repetida ("the the")
- "a" e "an" antes do som errado ("an test", "a hour")
- uma frase que começa com letra minúscula depois de um ponto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando as palavras ao redor deixam o erro claro
- "could of" em vez de "could have"
- dois espaços seguidos dentro de uma frase
- um espaço antes de vírgula, ponto ou outro sinal que pertence à palavra anterior
- nenhum espaço depois do fim de uma frase
- "alot", e "everyday" em vez de "every day" ("almost everyday")
- verbos que não concordam com o sujeito em casos claros ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- a forma errada de um verbo ("did went", "has went", "can't hardly", "don't have no")
- palavras trocadas ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" em vez de "whether", "chose" em vez de "choose", "advise" em vez de "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- uma vírgula depois do ano em uma data ("On June 5, 2020, we left"), uma vírgula depois de cidade e estado ("in Austin, Texas, the heat"), e um hífen em um composto antes de um substantivo ("a well-known author")
- um "i" minúsculo para a palavra "I", e os dias e meses em letra minúscula

Ele prefere deixar passar um erro a marcar um texto que está certo, então uma frase sem linha azul não é garantia de que esteja correta.

Clique com o botão direito no problema. O menu mostra o que está errado e depois uma ou mais sugestões. Clique em uma sugestão para colocá-la no texto, e pressione Ctrl+Z para desfazer. **Ignorar Este Problema** remove a linha somente deste ponto, até você fechar o JeffWrite. As mesmas palavras em outro lugar mantêm a linha, e editar outro trecho do parágrafo não a faz voltar. **Ignorar Tudo** remove a linha de todos os problemas com essa mensagem nessas palavras.

A verificação é feita no seu próprio computador, sem conexão com a internet, em segundo plano: a digitação nunca é interrompida, e um parágrafo é verificado logo depois que você para de digitar nele. Um parágrafo é verificado até os seus primeiros 100.000 caracteres. Um documento que exige mais verificação do que o limite deixa de ser verificado, e a barra de status informa isso uma vez; a verificação ortográfica continua funcionando.

**Quais idiomas são verificados.** A gramática é verificada somente em inglês, em todas as variantes regionais (americano, britânico, canadense, australiano e as demais). Um texto em qualquer outro idioma é verificado somente quanto à ortografia e não recebe marcas de gramática. Um parágrafo é verificado no idioma em que está marcado, e um texto sem idioma é verificado como inglês. Em um parágrafo com mais de um idioma, só o inglês é verificado, e o restante é ignorado. Textos que um arquivo marca como "não verificar" também são ignorados, assim como as palavras excluídas por uma alteração controlada, o texto oculto e os resultados de campos (como números de página). A gramática é verificada no texto do documento, e ainda não nas tabelas, nos cabeçalhos, nos rodapés e nas notas.

Para desligá-la, clique em **Revisão > Verificar Gramática ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**. É uma única opção para todos os aplicativos do JeffOffice, e é a mesma em todas as janelas. Quando ela está ativada, o painel Ortografia (F7) percorre os problemas de gramática depois da ortografia.

**Escolha o que ela procura.** Clique em **Arquivo > Opções > Revisão de Texto > Configurações de gramática** para escolher os tipos de problema: **Gramática** (verbos que não concordam, forma errada de um verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífens extras ou ausentes), **Maiúsculas e minúsculas**, **Palavras trocadas** (palavras que soam parecido), **Repetição** e **Estilo**. **Estilo** fica desativado até você ligá-lo. Ele faz sugestões e não aponta erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam com a mesma palavra e expressões prolixas ("in order to", "due to the fact that"). A escolha é mantida quando você clica em **OK** nas Opções. Ela se aplica de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas deste aplicativo, e aos outros aplicativos do JeffOffice na próxima vez que forem iniciados.

## Idiomas

O JeffWrite verifica cada trecho de texto no seu próprio idioma. Um arquivo pode informar em que idioma está o texto, até uma única palavra, e o JeffWrite lê essa informação. Um texto que não informa nada é verificado no idioma do documento. O arquivo indica esse idioma para o documento inteiro, e um documento novo usa o inglês americano.

Um texto em um idioma para o qual o JeffWrite não tem dicionário não é verificado. Também não é verificado um texto que um arquivo marca como "não verificar ortografia". Você ainda não pode mudar o idioma de um texto no JeffWrite, mas ele mantém o que o arquivo informa.

Os 31 dicionários são: inglês (americano, britânico, canadense e australiano), espanhol (21 regiões, incluindo México e Espanha), francês, alemão, italiano, holandês e português (Brasil e Portugal).

## O que ainda não é verificado

As linhas vermelhas aparecem no texto, nas células das tabelas e em um cabeçalho, rodapé ou nota de rodapé enquanto você os edita. Elas ainda não aparecem nas caixas de texto, mas o painel Ortografia percorre essas caixas.

## Veja também

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
